#include <ForgeSim/Core/Application.hpp>
#include <ForgeSim/Core/Assert.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>

#include <chrono>
#include <concepts>
#include <utility>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

static_assert(
	std::same_as<
	ForgeSim::Core::Timer::Duration,
	std::chrono::nanoseconds
	>
);

static_assert(
	std::same_as<
	decltype(ForgeSim::Core::Timer().Elapsed()),
	ForgeSim::Core::Timer::Duration
	>
);

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
#if !defined(NDEBUG)

	if (argc > 1 && std::string_view(argv[1]) == "--assert-failure")
	{
		FORGESIM_ASSERT(false);
	}
#endif

	std::ostringstream infoOutput;
	std::ostringstream errorOutput;

	std::streambuf* const originalClogBuffer =
		std::clog.rdbuf(infoOutput.rdbuf());

	std::streambuf* const originalCerrBuffer =
		std::cerr.rdbuf(errorOutput.rdbuf());

	ForgeSim::Core::LogInfo("Info test message");
	ForgeSim::Core::LogWarning("Warning test message");
	ForgeSim::Core::LogError("Error test message");

	std::clog.rdbuf(originalClogBuffer);
	std::cerr.rdbuf(originalCerrBuffer);

	const std::string infoText = infoOutput.str();
	const std::string errorText = errorOutput.str();

	const bool infoPassed = 
		infoText.find("[INFO]") != std::string::npos &&	
		infoText.find("Info test message") != std::string::npos &&
		infoText.find("CoreSmokeTests.cpp") != std::string::npos;

	const bool warningPassed =
		errorText.find("[WARNING]") != std::string::npos &&
		errorText.find("Warning test message") != std::string::npos &&
		errorText.find("CoreSmokeTests.cpp") != std::string::npos;
	
	const bool errorPassed = 
		errorText.find("[ERROR]") != std::string::npos &&	
		errorText.find("Error test message") != std::string::npos &&
		errorText.find("CoreSmokeTests.cpp") != std::string::npos;
	
	int assertionEvaluationCount = 0;

	FORGESIM_ASSERT(++assertionEvaluationCount == 1);

#if defined(NDEBUG)
	const bool assertionPassed = 
		assertionEvaluationCount == 0;
#else
	const bool assertionPassed =
		assertionEvaluationCount == 1;
#endif

	ForgeSim::Core::Timer timer;

	const auto firstElapsed = timer.Elapsed();
	const auto secondElapsed = timer.Elapsed();

	const bool monotonicPassed = 
		firstElapsed >= ForgeSim::Core::Timer::Duration(0) &&
		secondElapsed >= firstElapsed;

	timer.Reset();

	const auto firstElapsedAfterReset = timer.Elapsed();
	const auto secondElapsedAfterReset = timer.Elapsed();

	const bool resetPassed =
		firstElapsedAfterReset >= ForgeSim::Core::Timer::Duration(0) &&
		secondElapsedAfterReset >= firstElapsedAfterReset;

	if (!infoPassed ||
		!warningPassed ||
		!errorPassed ||
		!assertionPassed ||
		!monotonicPassed ||
		!resetPassed
		)
	{
		std::cerr << "ForgeSim Core smoke test failed \n";
		return EXIT_FAILURE;
	}

	std::cout << "ForgeSim Core smoke test passed \n";
	return EXIT_SUCCESS;
}