#include <ForgeSim/Core/RuntimeDiagnostics.hpp>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>

int main()
{
	using namespace std::chrono_literals;

	using ForgeSim::Core::FixedStepScheduler;
	using ForgeSim::Core::RuntimeDiagnostics;

	constexpr double tolerance = 1e-6;

	// Ten supplied 100 ms frames should produce one
	// one-second diagnostics snapshot.
	{
		RuntimeDiagnostics diagnostics;

		for (int frame = 0; frame < 9; ++frame)
		{
			const FixedStepScheduler::AddTimeResult result{
				.stepCount = 6,
				.droppedTime = {}
			};

			if (diagnostics.AddFrame(100ms, result))
			{
				std::cerr
					<< "Diagnostics produced a snapshot too early\n";

				return EXIT_FAILURE;
			}
		}

		const FixedStepScheduler::AddTimeResult finalResult{
			.stepCount = 6,
			.droppedTime = 20ms
		};

		const auto snapshot =
			diagnostics.AddFrame(
				100ms,
				finalResult);

		if (!snapshot)
		{
			std::cerr
				<< "Expected a diagnostics snapshot after one second\n";

			return EXIT_FAILURE;
		}

		if (std::abs(
			snapshot->frameStatistics.framesPerSecond -
			10.0) > tolerance)
		{
			std::cerr
				<< "Unexpected frame rate\n";

			return EXIT_FAILURE;
		}

		if (std::abs(
			snapshot->frameStatistics
			.averageFrameTimeMilliseconds -
			100.0) > tolerance)
		{
			std::cerr
				<< "Unexpected average frame time\n";

			return EXIT_FAILURE;
		}

		if (snapshot->fixedUpdateCount != 60)
		{
			std::cerr
				<< "Unexpected fixed-update count\n";

			return EXIT_FAILURE;
		}

		if (snapshot->catchUpLimitedFrameCount != 1)
		{
			std::cerr
				<< "Expected one catch-up-limited frame\n";

			return EXIT_FAILURE;
		}

		if (snapshot->droppedSimulationTime != 20ms)
		{
			std::cerr
				<< "Unexpected dropped simulation time\n";

			return EXIT_FAILURE;
		}
	}

	// Disabled diagnostics should neither accumulate nor
	// produce snapshots.
	{
		RuntimeDiagnostics diagnostics;
		diagnostics.SetEnabled(false);

		if (diagnostics.IsEnabled())
		{
			std::cerr
				<< "Diagnostics should be disabled\n";

			return EXIT_FAILURE;
		}

		const FixedStepScheduler::AddTimeResult result{
			.stepCount = 6,
			.droppedTime = {}
		};

		if (diagnostics.AddFrame(1s, result))
		{
			std::cerr
				<< "Disabled diagnostics produced a snapshot\n";

			return EXIT_FAILURE;
		}

		diagnostics.SetEnabled(true);

		const auto snapshot =
			diagnostics.AddFrame(1s, result);

		if (!snapshot)
		{
			std::cerr
				<< "Re-enabled diagnostics did not produce a snapshot\n";

			return EXIT_FAILURE;
		}

		if (snapshot->fixedUpdateCount != 6)
		{
			std::cerr
				<< "Disabled diagnostics retained stale data\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}