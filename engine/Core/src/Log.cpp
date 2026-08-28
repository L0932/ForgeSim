#include <ForgeSim/Core/Log.hpp>

#include <iostream>
#include <ostream>
#include <syncstream>

namespace
{
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
	void Log(
		LogLevel level,
		std::string_view message,
		const std::source_location& location
	)
	{
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