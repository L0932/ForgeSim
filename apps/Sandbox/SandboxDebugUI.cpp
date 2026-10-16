#include "SandboxDebugUI.hpp"
#include "SandboxObject.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/trigonometric.hpp>

#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>
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

	void SandboxDebugUI::Draw(const SandboxDebugUIContext& context)
	{
		SandboxObject* selectedObject =
			context.selectedObject;

		const Core::RuntimeDiagnosticsSnapshot*
			diagnosticsSnapshot =
			context.diagnosticsSnapshot;

		const Renderer::PerspectiveCamera& camera =
			context.camera;

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

		ImGui::SeparatorText("Camera");

		const glm::vec3& cameraPosition =
			camera.Position();

		const glm::vec3 cameraForward =
			camera.ForwardDirection();

		const glm::vec3 cameraTarget =
			cameraPosition + cameraForward;

		ImGui::Text(
			"Position: %.3f, %.3f, %.3f",
			cameraPosition.x,
			cameraPosition.y,
			cameraPosition.z);

		ImGui::Text(
			"Forward: %.3f, %.3f, %.3f",
			cameraForward.x,
			cameraForward.y,
			cameraForward.z);

		if (ImGui::Button("Copy camera pose"))
		{
			std::array<char, 512> cameraPose{};

			std::snprintf(
				cameraPose.data(),
				cameraPose.size(),
				"glm::vec3{ %.3ff, %.3ff, %.3ff },\n"
				"glm::vec3{ %.3ff, %.3ff, %.3ff },",
				cameraPosition.x,
				cameraPosition.y,
				cameraPosition.z,
				cameraTarget.x,
				cameraTarget.y,
				cameraTarget.z);

			ImGui::SetClipboardText(
				cameraPose.data());
		}

		ImGui::SeparatorText("Rendering");

		ImGui::Checkbox(
			"Show environment background",
			&m_ShowEnvironmentBackground);

		ImGui::BeginDisabled(
			m_ShowEnvironmentBackground);

		ImGui::ColorEdit3(
			"Fallback background color",
			&m_BackgroundColor.x,
			ImGuiColorEditFlags_Float);

		ImGui::EndDisabled();

		ImGui::SliderFloat(
			"Exposure",
			&m_Exposure,
			0.05f,
			5.0f, 
			"%.2f",
			ImGuiSliderFlags_Logarithmic);
		
		ImGui::SliderFloat(
			"Environment intensity",
			&m_EnvironmentIntensity,
			0.0f,
			2.0f,
			"%.2f");

		ImGui::SeparatorText("Directional Light");

		DirectionalLight& light =
			context.directionalLight;

		ImGui::Text(
			"Light ID: %llu",
			static_cast<unsigned long long>(light.id));

		ImGui::Checkbox(
			"Light enabled",
			&light.enabled
		);

		ImGui::ColorEdit3(
			"Light color",
			&light.color.x,
			ImGuiColorEditFlags_Float);

		ImGui::DragFloat(
			"Light intensity",
			&light.intensity,
			0.05f,
			0.0f,
			100.0f,
			"%.2f",
			ImGuiSliderFlags_AlwaysClamp);

		glm::vec3 directionToLight =
			light.directionToLight;

		if (ImGui::DragFloat3(
			"Direction to light",
			&directionToLight.x,
			0.01f,
			-1.0f,
			1.0f,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp))
		{
			const float lengthSquared =
				glm::dot(directionToLight, directionToLight);

			if (std::isfinite(lengthSquared) &&
				lengthSquared > 1e-12f)
			{
				light.directionToLight =
					directionToLight / std::sqrt(lengthSquared);
			}
		}

		ImGui::TextDisabled(
			"World-space direction from the surface toward the light.");

		ImGui::SeparatorText("Selection");

		ImGui::Checkbox(
			"Show selection highlight",
			&m_SelectionHighlightEnabled);

		ImGui::BeginDisabled(
			!m_SelectionHighlightEnabled);

		ImGui::ColorEdit3(
			"Highlight color",
			&m_SelectionHighlightColor.x);

		ImGui::SliderFloat(
			"Highlight strength",
			&m_SelectionHighlightStrength,
			0.0f,
			1.0f,
			"%.2f");

		ImGui::EndDisabled();

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
			const glm::vec3 modelExtent =
				context.modelBounds.maximum -
				context.modelBounds.minimum;

			float positionDragSpeed = 0.001f;

			const bool hasValidExtent =
				std::isfinite(modelExtent.x) &&
				std::isfinite(modelExtent.y) &&
				std::isfinite(modelExtent.z) &&
				modelExtent.x >= 0.0f &&
				modelExtent.y >= 0.0f &&
				modelExtent.z >= 0.0f;

			if (hasValidExtent)
			{
				const float characteristicSize =
					std::max({
						modelExtent.x,
						modelExtent.y,
						modelExtent.z
						});

				if (characteristicSize > 0.0f)
				{
					positionDragSpeed =
						std::clamp(
							characteristicSize * 0.001f,
							0.000001f,
							10.0f);
				}
			}

			ImGui::Text(
				"Selected objectID: %llu",
				static_cast<unsigned long long>(
					selectedObject->id));

			ImGui::SeparatorText("Transform");

			ImGui::DragFloat3(
				"Position",
				&selectedObject->transform.position.x,
				positionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

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

	glm::vec3 SandboxDebugUI::BackgroundColor()
		const noexcept
	{
		return m_BackgroundColor;
	}

	float SandboxDebugUI::Exposure() const noexcept
	{
		return m_Exposure;
	}

	float SandboxDebugUI::EnvironmentIntensity()
		const noexcept
	{
		return m_EnvironmentIntensity;
	}

	bool SandboxDebugUI::ShowEnvironmentBackground()
		const noexcept
	{
		return m_ShowEnvironmentBackground;
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

	bool SandboxDebugUI::SelectionHighlightEnabled()
		const noexcept
	{
		return m_SelectionHighlightEnabled;
	}

	const glm::vec3&
		SandboxDebugUI::SelectionHighlightColor()
		const noexcept
	{
		return m_SelectionHighlightColor;
	}

	float SandboxDebugUI::SelectionHighlightStrength()
		const noexcept
	{
		return m_SelectionHighlightStrength;
	}
}