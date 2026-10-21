#include "SandboxDebugUI.hpp"
#include "SandboxScene.hpp"
#include "SandboxObject.hpp"
#include "SceneLightEditing.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
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
	namespace
	{
		[[nodiscard]] float CalculateLightPositionDragSpeed(
			const Renderer::AxisAlignedBoundingBox& bounds)
		{
			const glm::vec3 extent =
				bounds.maximum - bounds.minimum;

			if (!std::isfinite(extent.x) ||
				!std::isfinite(extent.y) ||
				!std::isfinite(extent.z) ||
				extent.x < 0.0f ||
				extent.y < 0.0f ||
				extent.z < 0.0f)
			{
				return 0.001f;
			}

			const float characteristicSize =
				std::max({ extent.x, extent.y, extent.z });

			if (characteristicSize <= 0.0f)
			{
				return 0.001f;
			}

			return std::clamp(
				characteristicSize * 0.001f,
				0.000001f,
				10.0f);
		}
	}

	SandboxDebugUI::SandboxDebugUI(
		Platform::GlfwWindow& window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

	TransformEditorUIState SandboxDebugUI::DrawTransformEditorControls(
		bool editingActive,
		bool navigationActive)
	{
		auto& state = m_TransformEditorState;

		state.undoRequested = false;
		state.redoRequested = false;

		const ImGuiIO& io = ImGui::GetIO();

		const bool operationShortcutsAllowed =
			!editingActive &&
			!navigationActive &&
			!io.WantTextInput &&
			!ImGui::IsAnyItemActive() &&
			!io.KeyCtrl &&
			!io.KeyAlt &&
			!io.KeyShift;

		if (operationShortcutsAllowed)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_W, false))
			{
				state.operation = GizmoOperation::Translate;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_E, false))
			{
				state.operation = GizmoOperation::Rotate;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_R, false))
			{
				state.operation = GizmoOperation::Scale;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_X, false))
			{
				state.space =
					state.space == GizmoSpace::World
					? GizmoSpace::Local
					: GizmoSpace::World;
			}
		}

		if (ImGui::Begin("Transform Gizmo"))
		{
			// Keep the operation and coordinate space stable
			// throughout an active drag.
			ImGui::BeginDisabled(editingActive);

			if (ImGui::RadioButton(
				"Translate (W)",
				state.operation == GizmoOperation::Translate))
			{
				state.operation = GizmoOperation::Translate;
			}

			ImGui::SameLine();

			if (ImGui::RadioButton(
				"Rotate (E)",
				state.operation == GizmoOperation::Rotate))
			{
				state.operation = GizmoOperation::Rotate;
			}

			ImGui::SameLine();

			if (ImGui::RadioButton(
				"Scale (R)",
				state.operation == GizmoOperation::Scale))
			{
				state.operation = GizmoOperation::Scale;
			}

			ImGui::BeginDisabled(
				state.operation == GizmoOperation::Scale);

			if (ImGui::RadioButton(
				"World",
				state.space == GizmoSpace::World))
			{
				state.space = GizmoSpace::World;
			}

			ImGui::SameLine();

			if (ImGui::RadioButton(
				"Local",
				state.space == GizmoSpace::Local))
			{
				state.space = GizmoSpace::Local;
			}

			ImGui::EndDisabled();
			ImGui::EndDisabled();

			if (state.operation == GizmoOperation::Scale)
			{
				ImGui::TextDisabled("Scale uses local axes.");
			}
		}

		ImGui::End();

		// Let active UI controls handle their own keyboard input.
		if (!editingActive &&
			!ImGui::IsAnyItemActive() &&
			!io.WantTextInput &&
			io.KeyCtrl &&
			!io.KeyAlt)
		{
			const bool zPressed =
				ImGui::IsKeyPressed(ImGuiKey_Z, false);

			state.undoRequested =
				!io.KeyShift && zPressed;

			state.redoRequested =
				(io.KeyShift && zPressed) ||
				(!io.KeyShift &&
					ImGui::IsKeyPressed(ImGuiKey_Y, false));
		}

		return state;
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

		// Submit before any windows that may dock into this workspace.
		// The empty center remains transparent to the scene behind it.
		ImGui::DockSpaceOverViewport(
			0,
			ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_PassthruCentralNode);

		if (ImGui::IsKeyPressed(
			ImGuiKey_F1,
			false))
		{
			m_IsVisible = !m_IsVisible;
		}
	}

	void SandboxDebugUI::Draw(const SandboxDebugUIContext& context)
	{
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

		ImGui::Checkbox(
			"Show scene lights",
			&m_ShowLights);

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

		ImGui::SeparatorText("Scene Lights");

		if (ImGui::Selectable(
			"None",
			context.selection.Empty()))
		{
			context.selection.Clear();
		}

		const auto drawLightEntry =
			[&](const char* typeName,
				std::size_t displayNumber,
				SceneLightId id)
			{
				std::array<char, 128> label{};

				// Text before ## is displayed; the stable ID after it
				// identifies the widget independently of display numbering.
				std::snprintf(
					label.data(),
					label.size(),
					"%s %zu##SceneLight_%llu",
					typeName,
					displayNumber,
					static_cast<unsigned long long>(id));

				if (ImGui::Selectable(
					label.data(), 
					context.selection.IsLightSelected(id)))
				{
					context.selection.SelectLight(id);
				}
			};

		drawLightEntry(
			"Directional",
			1,
			context.directionalLight.id);

		std::size_t pointNumber = 1;

		for (const PointLight& light : context.pointLights)
		{
			drawLightEntry(
				"Point",
				pointNumber++,
				light.id);
		}

		std::size_t spotNumber = 1;

		for (const SpotLight& light : context.spotLights)
		{
			drawLightEntry(
				"Spot",
				spotNumber++,
				light.id);
		}

		if (m_LightValidationError &&
			!context.selection.IsLightSelected(
				m_LightValidationError->lightId))
		{
			m_LightValidationError.reset();
		}

		const float lightPositionDragSpeed =
			CalculateLightPositionDragSpeed(context.modelBounds);

		if (context.selection.IsLightSelected(
			context.directionalLight.id))
		{
			ImGui::SeparatorText("Directional Light");

			DirectionalLight& target = context.directionalLight;
			DirectionalLight candidate = target;

			ImGui::Text(
				"Light ID: %llu",
				static_cast<unsigned long long>(target.id));

			bool changed = false;

			changed |= ImGui::Checkbox(
				"Light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Light editor position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat3(
				"Direction to light",
				&candidate.directionToLight.x,
				0.01f,
				0.0f,
				0.0f,
				"%.3f");

			ImGui::TextDisabled(
				"Position moves the editor marker, not the illumination.");

			ImGui::TextDisabled(
				"Direction points from the surface toward the light.");

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}
		}

		for (PointLight& target : context.pointLights)
		{
			if (!context.selection.IsLightSelected(target.id))
			{
				continue;
			}

			ImGui::SeparatorText("Selected Point Light");

			PointLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Point light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Point light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Point light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Point light position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat(
				"Point light range",
				&candidate.range,
				lightPositionDragSpeed,
				0.000001f,
				std::numeric_limits<float>::max(),
				"%.6f",
				ImGuiSliderFlags_AlwaysClamp |
				ImGuiSliderFlags_NoRoundToFormat);

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}

			break;
		}

		for (SpotLight& target : context.spotLights)
		{
			if (!context.selection.IsLightSelected(target.id))
			{
				continue;
			}

			ImGui::SeparatorText("Selected Spot Light");

			SpotLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Spot light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Spot light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Spot light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Spot light position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat3(
				"Emission direction",
				&candidate.emissionDirection.x,
				0.01f,
				0.0f,
				0.0f,
				"%.3f");

			changed |= ImGui::DragFloat(
				"Spot light range",
				&candidate.range,
				lightPositionDragSpeed,
				0.000001f,
				std::numeric_limits<float>::max(),
				"%.6f",
				ImGuiSliderFlags_AlwaysClamp |
				ImGuiSliderFlags_NoRoundToFormat);

			float innerConeDegrees =
				glm::degrees(candidate.innerConeAngleRadians);

			float outerConeDegrees =
				glm::degrees(candidate.outerConeAngleRadians);

			if (ImGui::DragFloat(
				"Inner cone half-angle",
				&innerConeDegrees,
				0.1f,
				0.0f,
				0.0f,
				"%.2f deg"))
			{
				candidate.innerConeAngleRadians =
					glm::radians(innerConeDegrees);

				changed = true;
			}

			if (ImGui::DragFloat(
				"Outer cone half-angle",
				&outerConeDegrees,
				0.1f,
				0.0f,
				0.0f,
				"%.2f deg"))
			{
				candidate.outerConeAngleRadians =
					glm::radians(outerConeDegrees);

				changed = true;
			}

			ImGui::TextDisabled(
				"Angles are measured from the emission axis.");

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}

			break;
		}

		if (m_LightValidationError)
		{
			const LightValidationError& error =
				*m_LightValidationError;

			ImGui::TextWrapped(
				"Light %llu - %.*s: %.*s",
				static_cast<unsigned long long>(error.lightId),
				static_cast<int>(error.property.size()),
				error.property.data(),
				static_cast<int>(error.message.size()),
				error.message.data());
		}

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

		SandboxObject* selectedObject = nullptr;

		if (const auto objectId =
			context.selection.SelectedObjectId())
		{
			selectedObject =
				context.scene.FindObject(*objectId);

			if (selectedObject == nullptr)
			{
				context.selection.Clear();
			}
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

	bool SandboxDebugUI::ShowLights() const noexcept
	{
		return m_ShowLights;
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