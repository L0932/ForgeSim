#include "SandboxApplication.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <string>

#include <glad/gl.h>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Sandbox
{
	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
	{}

	void SandboxApplication::Run()
	{
		assert(
			!m_RunAttempted &&
			"Run() can only be called once per application instance");

		m_RunAttempted = true;

		m_Window.Show();

		glClearColor(0.05f, 0.15f, 0.30f, 1.0f);

		ForgeSim::Core::Timer timer;
		ForgeSim::Core::FrameStatistics frameStats;

		using FixedStepScheduler =
			ForgeSim::Core::FixedStepScheduler;

		const auto fixedStep =
			std::chrono::duration_cast<
			FixedStepScheduler::Duration>(
				std::chrono::seconds{ 1 }) / 60;

		constexpr std::uint32_t maximumStepsPerFrame = 4;

		FixedStepScheduler scheduler{
			fixedStep,
			maximumStepsPerFrame
		};

		std::uint64_t fixedUpdateCount = 0;

		while (!m_Window.ShouldClose())
		{
			m_Window.PollEvents();

			const auto frameDelta = timer.Restart();

			const auto fixedStepResult =
				scheduler.AddTime(frameDelta);

			for (std::uint32_t step = 0;
				step < fixedStepResult.stepCount;
				++step)
			{
				// Perform fixed-step update logic here.
				++fixedUpdateCount;
			}

			if (fixedStepResult.droppedTime >
				FixedStepScheduler::Duration::zero())
			{
				const double droppedMilliseconds =
					std::chrono::duration<double, std::milli>{
						fixedStepResult.droppedTime
				}.count();

				ForgeSim::Core::LogInfo(
					"Dropped simulation time: " +
					std::to_string(droppedMilliseconds) +
					"ms");
			}

			if (const auto snapshot =
				frameStats.AddFrame(frameDelta))
			{
				ForgeSim::Core::LogInfo(
					"FPS: " +
					std::to_string(snapshot->framesPerSecond) +
					", Average Frame Time: " +
					std::to_string(
						snapshot->averageFrameTimeMilliseconds) +
					" ms | Fixed updates: " +
					std::to_string(fixedUpdateCount));

				fixedUpdateCount = 0;
			}

			glClear(GL_COLOR_BUFFER_BIT);
			m_Window.SwapBuffers();
		}
	}
}