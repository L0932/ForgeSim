#pragma once

#include <source_location>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <string_view>

namespace ForgeSim::Core
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

    inline constexpr std::size_t LogHistoryCapacity = 1024;

    struct LogEntry
    {
        std::uint64_t sequence = 0;
        LogLevel level = LogLevel::Info;
        std::string message;
        std::string sourceFile;
        std::uint_least32_t sourceLine = 0;
    };

    // A bounded, process-local history. Terminal logging remains unchanged.
    // Returns owned copies newer than the consumer's cursor; safe on any thread.
    [[nodiscard]] std::vector<LogEntry> ReadLogEntries(std::uint64_t afterSequence = 0);

	void Log(
		LogLevel level, 
		std::string_view message, 
		const std::source_location& location = std::source_location::current()
	);

	inline void LogInfo(
		std::string_view message,
		const std::source_location& location = std::source_location::current()
	)
	{
		Log(LogLevel::Info, message, location);
	}

	inline void LogWarning(
		std::string_view message,
		const std::source_location& location = std::source_location::current()
	)
	{
		Log(LogLevel::Warning, message, location);
	}

	inline void LogError(
		std::string_view message,
		const std::source_location& location = std::source_location::current()
	)
	{
		Log(LogLevel::Error, message, location);
	}
}
