#include <ForgeSim/Core/Log.hpp>

#include <iostream>
#include <deque>
#include <mutex>
#include <utility>
#include <ostream>
#include <syncstream>

namespace
{
    struct LogHistory
    {
        std::mutex mutex;
        std::deque<ForgeSim::Core::LogEntry> entries;
        std::uint64_t sequence = 0;
    };

    LogHistory& History()
    {
        static LogHistory history;
        return history;
    }

    std::string BoundedMessage(std::string_view message)
    {
        constexpr std::size_t maximum = 8192;
        if (message.size() <= maximum) return std::string(message);
        std::size_t end = maximum;
        // Avoid cutting a UTF-8 character at the history's byte limit.
        while (end > 0 && (static_cast<unsigned char>(message[end]) & 0xc0) == 0x80) --end;
        return std::string(message.substr(0, end)) + "... [truncated]";
    }

	[[nodiscard]] std::string_view ToString(ForgeSim::Core::LogLevel level) noexcept
	{
		using ForgeSim::Core::LogLevel;

		switch (level)
		{
		case ForgeSim::Core::LogLevel::Info:
			return "INFO";
		case ForgeSim::Core::LogLevel::Warning:
			return "WARNING";
		case ForgeSim::Core::LogLevel::Error:
			return "ERROR";
		default:
			return "UNKNOWN";
		}
	}
}

namespace ForgeSim::Core
{
    std::vector<LogEntry> ReadLogEntries(std::uint64_t afterSequence)
    {
        auto& history = History();
        std::lock_guard lock(history.mutex);
        std::vector<LogEntry> result;
        if (history.entries.empty() || history.entries.back().sequence <= afterSequence)
            return result;
        for (const auto& entry : history.entries)
            if (entry.sequence > afterSequence) result.push_back(entry);
        return result;
    }

	void Log(
		LogLevel level,
		std::string_view message,
		const std::source_location& location
	)
	{
        {
            auto& history = History();
            std::lock_guard lock(history.mutex);
            LogEntry entry{history.sequence + 1, level, BoundedMessage(message),
                location.file_name(), location.line()};
            history.entries.push_back(std::move(entry));
            ++history.sequence;
            if (history.entries.size() > LogHistoryCapacity) history.entries.pop_front();
        }

		std::ostream& stream = 
			(level == LogLevel::Info) ? std::clog : std::cerr;

		std::osyncstream output(stream);

		output <<
			'[' << ToString(level) << "] "
			<< message
			<< " (" << location.file_name()
			<< ':' << location.line()
			<< ")\n";
	}
}