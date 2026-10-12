#include "SandboxDebugUI.hpp"
#include "SandboxObject.hpp"

#include <chrono>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/trigonometric.hpp>

#include <ForgeSim/Core/RuntimeDiagnostics.hpp>
#include <ForgeSim/Platform/GlfwWindow.hpp>

namespace ForgeSim::Sandbox
{
	SandboxDebugUI::SandboxDebugUI(
		Platform::GlfwWindow& window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		if (!ImGui_ImplGlfw_InitForOpenGL(
			window.NativeHandle(),
			true))
		{
			ImGui::DestroyContext();

			throw std::runtime_error(
				"Failed to initialize the Dear ImGui "
				"GLFW backend.");
		}

		if (!ImGui_ImplOpenGL3_Init(
			"#version 460 core"))
		{
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			throw std::runtime_error(
				"Failed to initialize the Dear ImGui "
				"OpenGL backend.");
		}
	}

	SandboxDebugUI::~SandboxDebugUI()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void SandboxDebugUI::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::IsKeyPressed(
			ImGuiKey_F1,
			false))
		{
			m_IsVisible = !m_IsVisible;
		}
	}

	void SandboxDebugUI::Draw(
		SandboxObject* selectedObject, 
		const Core::RuntimeDiagnosticsSnapshot*
		diagnosticsSnapshot)
	{
		if (!m_IsVisible)
		{
			return;
		}

		ImGui::Begin(
			"ForgeSim Debug",
			&m_IsVisible);

		ImGui::TextUnformatted(
			"ForgeSim Sandbox debug interface");

		ImGui::TextUnformatted(
			"Press F1 to show or hide this panel.");

		ImGui::Separator();

		ImGui::Checkbox(
			"Show reference grid",
			&m_ShowGrid);

		ImGui::Checkbox(
			"Show object bounds",
			&m_ShowObjectBounds);

		ImGui::SeparatorText("Runtime Diagnostics");

		ImGui::Checkbox(
			"Enable runtime diagnostics",
			&m_RuntimeDiagnosticsEnabled);

		if (!m_RuntimeDiagnosticsEnabled)
		{
			ImGui::TextDisabled(
				"Runtime diagnostics are disabled.");
		}
		else if (diagnosticsSnapshot == nullptr)
		{
			ImGui::TextDisabled(
				"Collecting runtime diagnostics...");
		}
		else
		{
			const double droppedMilliseconds =
				std::chrono::duration<double, std::milli>{
					diagnosticsSnapshot->droppedSimulationTime
			}.count();

			ImGui::Text(
				"Frame rate: %.1f FPS",
				diagnosticsSnapshot
				->frameStatistics
				.framesPerSecond);

			ImGui::Text(
				"Average frame time: %.3f ms",
				diagnosticsSnapshot
				->frameStatistics
				.averageFrameTimeMilliseconds);

			ImGui::Text(
				"Frames sampled: %llu",
				static_cast<unsigned long long>(
					diagnosticsSnapshot
					->frameStatistics
					.frameCount));

			ImGui::Text(
				"Fixed updates: %llu",
				static_cast<unsigned long long>(
					diagnosticsSnapshot
					->fixedUpdateCount));

			ImGui::Text(
				"Catch-up limit events: %llu",
				static_cast<unsigned long long>(
					diagnosticsSnapshot
					->catchUpLimitedFrameCount));

			ImGui::Text(
				"Dropped simulation time: %.3f ms",
				droppedMilliseconds);
		}

		ImGui::SeparatorText("Selected Object");

		if (selectedObject == nullptr)
		{
			ImGui::TextUnformatted("No object selected.");
		}
		else
		{
			ImGui::Text(
				"Selected objectID: %llu",
				static_cast<unsigned long long>(
					selectedObject->id));

			ImGui::SeparatorText("Transform");

			ImGui::DragFloat3(
				"Position",
				&selectedObject->transform.position.x,
				0.05f,
				0.0f,
				0.0f,
				"%.2f");

			glm::vec3 rotationDegrees =
				glm::degrees(
					selectedObject->transform.rotationRadians);

			if (ImGui::DragFloat3(
				"Rotation",
				&rotationDegrees.x,
				1.0f,
				0.0f,
				0.0f,
				"%.1f deg"))
			{
				selectedObject->transform.rotationRadians =
					glm::radians(rotationDegrees);
			}
		}

		ImGui::End();
	}

	void SandboxDebugUI::EndFrame()
	{
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData());
	}

	bool SandboxDebugUI::WantsMouseCapture()
		const noexcept
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool SandboxDebugUI::WantsKeyboardCapture()
		const noexcept
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool SandboxDebugUI::ShowGrid() const noexcept
	{
		return m_ShowGrid;
	}

	bool SandboxDebugUI::ShowObjectBounds() const noexcept
	{
		return m_ShowObjectBounds;
	}

	bool SandboxDebugUI::RuntimeDiagnosticsEnabled()
		const noexcept
	{
		return m_RuntimeDiagnosticsEnabled;
	}
}