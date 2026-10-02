#include "SandboxApplication.hpp"

#include <cassert>
#include <string>

#include <glad/gl.h>

#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>

namespace ForgeSim::Sandbox
{
	SandboxApplication::SandboxApplication(const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
	{
	}

	void SandboxApplication::Run()
	{
		assert(!m_RunAttempted && "Run() can only be called once per application instance");
		m_RunAttempted = true;

		m_Window.Show();

		glClearColor(0.05f, 0.15f, 0.30f, 1.0f);

		ForgeSim::Core::Timer timer;
		ForgeSim::Core::FrameStatistics frameStats;

		while (!m_Window.ShouldClose())
		{
			m_Window.PollEvents();
			
			const auto frameDelta = timer.Restart();
			
			if (const auto snapshot = frameStats.AddFrame(frameDelta))
			{
				ForgeSim::Core::LogInfo(
					"FPS: " + std::to_string(snapshot->framesPerSecond) +
					", Average Frame Time: " + std::to_string(snapshot->averageFrameTimeMilliseconds) + " ms"
				);
			}

			glClear(GL_COLOR_BUFFER_BIT);
			m_Window.SwapBuffers();
		}
	}
}