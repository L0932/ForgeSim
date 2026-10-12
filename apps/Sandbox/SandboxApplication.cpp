#include "SandboxApplication.hpp"
#include "FreeCameraController.hpp"
#include "ViewportSelectionController.hpp"

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_Scene()
		, m_Renderer()
		, m_DebugUI(m_Window)
	{
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

		ForgeSim::Renderer::PerspectiveCamera camera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		FreeCameraController cameraController;
		ViewportSelectionController selectionController;

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

			const ForgeSim::Platform::InputState& input =
				m_Window.GetInputState();

			const auto framebufferExtent =
				m_Window.GetFramebufferExtent();

			if (!framebufferExtent.IsDrawable())
			{
				m_Window.WaitEvents();
				static_cast<void>(timer.Restart());
				continue;
			}

			m_DebugUI.BeginFrame();

			const bool uiWantsMouse =
				m_DebugUI.WantsMouseCapture();

			const bool uiWantsKeyboard =
				m_DebugUI.WantsKeyboardCapture();

			if (const auto requestedCursorMode =
				cameraController.RequestedCursorMode(input))
			{
				const bool isReleaseRequest =
					*requestedCursorMode ==
					ForgeSim::Platform::CursorMode::Normal;

				if (!uiWantsMouse || isReleaseRequest)
				{
					m_Window.SetCursorMode(requestedCursorMode.value());
				}
			}

			const float aspectRatio =
				static_cast<float>(framebufferExtent.width) /
				static_cast<float>(framebufferExtent.height);

			const auto frameDelta = timer.Restart();

			const float deltaSeconds =
				std::chrono::duration<float>{ frameDelta }.count();

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

			if (!uiWantsMouse &&
				!uiWantsKeyboard)
			{
				cameraController.Update(
					camera,
					input,
					deltaSeconds);
			}

			const glm::mat4 view =
				camera.ViewMatrix();

			const glm::mat4 projection =
				camera.ProjectionMatrix(aspectRatio);

			if (!uiWantsMouse)
			{
				selectionController.Update(
					input,
					m_Window.GetWindowExtent(),
					framebufferExtent,
					view,
					projection,
					m_Scene.Objects()
				);
			}

			m_Renderer.Render(
				m_Scene.Objects(),
				selectionController.SelectedObjectId(),
				view,
				projection
			);

			SandboxObject* selectedObject = nullptr;

			if (const auto selectedObjectID =
				selectionController.SelectedObjectId())
			{
				selectedObject =
					m_Scene.FindObject(*selectedObjectID);
			}

			m_DebugUI.Draw(selectedObject);
			m_DebugUI.EndFrame();

			m_Window.SwapBuffers();
		}
	}
}