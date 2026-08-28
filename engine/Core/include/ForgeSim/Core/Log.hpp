#pragma once

#include <source_location>
#include <string_view>

namespace ForgeSim::Core
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

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
