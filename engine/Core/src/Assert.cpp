#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Assert.hpp>

#include <cstdlib>
#include <string>

namespace ForgeSim::Core
{
	[[noreturn]] void HandleAssertionFailure(
		std::string_view expression,
		const std::source_location& location
	)
	{
		LogError("Assertion failed: " + std::string(expression), location);
		std::abort();
	}
}