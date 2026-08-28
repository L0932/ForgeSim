#pragma once

#include <source_location>
#include <string_view>

namespace ForgeSim::Core
{
	[[noreturn]] void HandleAssertionFailure(
		std::string_view message,
		const std::source_location& location = std::source_location::current()
	);
}

#if defined(NDEBUG)

#define FORGESIM_ASSERT(expression) \
	static_cast<void>(0)
#else
#define FORGESIM_ASSERT(expression) \
	((expression) \
		? static_cast<void>(0) \
		: ForgeSim::Core::HandleAssertionFailure( \
			#expression, \
			std::source_location::current()))
#endif