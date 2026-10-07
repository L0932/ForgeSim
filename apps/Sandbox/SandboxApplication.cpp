#include "SandboxApplication.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <glad/gl.h>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		struct Vertex
		{
			float position[3];
			float color[3];
		};

		constexpr std::array<Vertex, 4> vertices{
			Vertex{
				.position = { -0.5f, -0.5f, 0.0f },
				.color = { 1.0f, 0.0f, 0.0f }
			},
			Vertex{
				.position = { 0.5f, -0.5f, 0.0f },
				.color = { 0.0f, 1.0f, 0.0f }
			},
			Vertex{
				.position = { 0.5f, 0.5f, 0.0f },
				.color = { 0.0f, 0.0f, 1.0f }
			},
			Vertex{
				.position = { -0.5f, 0.5f, 0.0f },
				.color = { 1.0f, 1.0f, 0.0f }
			}
		};

		constexpr std::array<std::uint32_t, 6> indices{
			0, 1, 2,
			2, 3, 0,
		};

		constexpr std::string_view vertexShaderSource = R"(
			#version 460 core
			layout(location = 0) in vec3 aPosition;
			layout(location = 1) in vec3 aColor;
			out vec3 vColor;
			void main()
			{
				gl_Position = vec4(aPosition, 1.0);
				vColor = aColor;
			}
		)";

		constexpr std::string_view fragmentShaderSource = R"(
			#version 460 core
			in vec3 vColor;
			out vec4 fragColor;
			void main()
			{
				fragColor = vec4(vColor, 1.0);
			}
		)";
	}

	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_ShaderProgram(
			vertexShaderSource, 
			fragmentShaderSource)
		, m_VertexBuffer(
			vertices.data(),
			vertices.size() * sizeof(Vertex))
		, m_IndexBuffer(
			indices.data(),
			indices.size() * sizeof(std::uint32_t))
		, m_VertexArray()
	{
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_VertexArray.SetVertexBuffer(
			m_VertexBuffer,
			vertexBindingIndex,
			sizeof(Vertex));

		m_VertexArray.SetIndexBuffer(m_IndexBuffer);

		m_VertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(Vertex, position));

		m_VertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(Vertex, color));
	}

	void SandboxApplication::Run()
	{
		if (m_RunAttempted)
		{
			throw std::logic_error(
				"Run() can only be called once per application instance.");
		}

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

			if (m_Window.ShouldClose())
			{
				break;
			}

			if (!m_Window.GetFramebufferExtent().IsDrawable())
			{
				m_Window.WaitEvents();
				continue;
			}

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

			m_ShaderProgram.Bind();
			m_VertexArray.Bind();

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(indices.size()),
				GL_UNSIGNED_INT,
				nullptr);

			m_Window.SwapBuffers();
		}
	}
}