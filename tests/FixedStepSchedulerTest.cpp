#include <ForgeSim/Core/FixedStepScheduler.hpp>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>

int main()
{
	using namespace std::chrono_literals;
	using ForgeSim::Core::FixedStepScheduler;

	constexpr double alphaTolerance = 1e-6;
	
	// Test 1: AddTime with 5ms on a 10ms fixed step should return 0 and interpolation alpha should be 0.5
	{
		FixedStepScheduler scheduler(10ms, 4);	

		const auto firstResult = scheduler.AddTime(5ms);

		if (firstResult.stepCount != 0)
		{
			std::cerr << "Expected zero steps after adding 5ms\n";
			return EXIT_FAILURE;
		}

		if (firstResult.droppedTime != 0ms)
		{
			std::cerr << "Expected no dropped time after adding 5ms\n";
			return EXIT_FAILURE;
		}


		if (std::abs(scheduler.InterpolationAlpha() - 0.5) > alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.5\n";

			return EXIT_FAILURE;
		}

		const auto secondResult = scheduler.AddTime(5ms);

		if (secondResult.stepCount != 1)
		{
			std::cerr << "Expected one step after adding another 5ms\n";
			return EXIT_FAILURE;
		}

		if (secondResult.droppedTime != 0ms)
		{
			std::cerr << "Expected no dropped time after one complete step\n";
			return EXIT_FAILURE;
		}

		if (std::abs(scheduler.InterpolationAlpha()) > alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.0\n";
			return EXIT_FAILURE;
		}
	}

	// Test 2: AddTime with 25ms on a 10ms fixed step should return 2 and interpolation alpha should be 0.5
	{
		FixedStepScheduler scheduler{ 10ms, 4 };

		const auto result = scheduler.AddTime(25ms);

		if (result.stepCount != 2)
		{
			std::cerr << "Expected two steps after adding 25ms\n";
			return EXIT_FAILURE;
		}

		if (result.droppedTime != 0ms)
		{
			std::cerr << "Expected no dropped time after adding 25ms\n";
			return EXIT_FAILURE;
		}

		if (std::abs(scheduler.InterpolationAlpha() - 0.5) >
			alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.5\n";
			return EXIT_FAILURE;
		}
	}

	// Test 3: AddTime with 105ms on a 10ms fixed step should return 4 (limited by max steps) and interpolation alpha should be 0.5
	{
		FixedStepScheduler scheduler{ 10ms, 4 };

		const auto result = scheduler.AddTime(105ms);

		if (result.stepCount != 4)
		{
			std::cerr << "Expected step count to be limited to four\n";
			return EXIT_FAILURE;
		}

		if (result.droppedTime != 60ms)
		{
			std::cerr << "Expected 60ms of dropped time\n";
			return EXIT_FAILURE;
		}

		if (std::abs(scheduler.InterpolationAlpha() - 0.5) >
			alphaTolerance)
		{
			std::cerr << "Expected the 5ms remainder to be preserved\n";
			return EXIT_FAILURE;
		}

		std::cout
			<< "Executed steps: " << result.stepCount
			<< "\nDropped time: " << result.droppedTime.count() << "ns"
			<< "\nInterpolation alpha: " << scheduler.InterpolationAlpha()
			<< '\n';
	}

	// Test 4: Reset should clear accumulated time and interpolation alpha should be 0.0
	{
		FixedStepScheduler scheduler{ 10ms, 4 };

		const auto firstResult = scheduler.AddTime(5ms);

		if (firstResult.stepCount != 0)
		{
			std::cerr << "Expected zero steps before reset\n";
			return EXIT_FAILURE;
		}

		if (std::abs(scheduler.InterpolationAlpha() - 0.5) >
			alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.5 before reset\n";
			return EXIT_FAILURE;
		}

		scheduler.Reset();

		if (std::abs(scheduler.InterpolationAlpha()) > alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.0 after reset\n";
			return EXIT_FAILURE;
		}

		const auto secondResult = scheduler.AddTime(5ms);

		if (secondResult.stepCount != 0)
		{
			std::cerr << "Expected reset to clear previously accumulated time\n";
			return EXIT_FAILURE;
		}

		if (std::abs(scheduler.InterpolationAlpha() - 0.5) >
			alphaTolerance)
		{
			std::cerr << "Expected interpolation alpha of 0.5 after new time\n";
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}