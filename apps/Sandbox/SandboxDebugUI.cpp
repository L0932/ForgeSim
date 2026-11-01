#include "SandboxDebugUI.hpp"
#include "SandboxScene.hpp"
#include "SceneCameraState.hpp"
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
#include <string>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/trigonometric.hpp>

#include <ForgeSim/Build/Version.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/RuntimeDiagnostics.hpp>
#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
    void SandboxDebugUI::ResetSceneControls()
    {
        m_SelectedBookmark.clear();
        m_BookmarkName.fill(0);
        m_EnvironmentAsset.fill(0);
        m_EnvironmentInputInitialized = false;
        m_PresentationMessage.clear();
    }

	SandboxDebugUI::SandboxDebugUI(
		Platform::GlfwWindow& window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		m_Workspace.Initialize();

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

		ImGui::Text(
			"ForgeSim Sandbox %s",
			ForgeSim::Build::Version);

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

		ImGui::BeginDisabled(context.cameraEditingDisabled);

		if (ImGui::Button("Save current viewpoint"))
		{
            const SceneCameraData saved = CaptureSceneCamera(
                context.camera, context.referenceViewport);

			if (const auto error = context.scene.TrySetSavedCamera(saved))
			{
				Core::LogError(
					std::string{ "Cannot save viewpoint at 'camera." } +
					std::string{ error->property } +
					"': " +
					std::string{ error->message });
			}
		}

		ImGui::BeginDisabled(!context.scene.SavedCamera());

		if (ImGui::Button("Clear saved viewpoint"))
		{
			context.scene.ClearSavedCamera();
		}

		ImGui::EndDisabled();
		ImGui::EndDisabled();

		ImGui::TextUnformatted(
			context.scene.SavedCamera()
			? "Scene has a saved viewpoint."
			: "Scene will use automatic framing.");

		ImGui::TextWrapped(
			"Save the scene file to keep viewpoint changes.");

        const auto reportAction = [&](auto&& action)
        {
            try
            {
                action();
                m_PresentationMessage.clear();
            }
            catch (const std::exception& error)
            {
                m_PresentationMessage = error.what();
            }
        };

        ImGui::SeparatorText("Camera bookmarks");
        ImGui::BeginDisabled(context.cameraEditingDisabled);
        ImGui::InputText("Bookmark name", m_BookmarkName.data(), m_BookmarkName.size());
        if (ImGui::Button("Add bookmark"))
        {
            reportAction([&]
            {
                context.scene.AddCameraBookmark(m_BookmarkName.data(),
                    CaptureSceneCamera(context.camera, context.referenceViewport));
                m_SelectedBookmark = m_BookmarkName.data();
            });
        }

        std::optional<std::size_t> requestedBookmark;
        if (ImGui::BeginCombo("Bookmarks",
            m_SelectedBookmark.empty() ? "Select a bookmark" : m_SelectedBookmark.c_str()))
        {
            const auto bookmarks = context.scene.CameraBookmarks();
            for (std::size_t index = 0; index < bookmarks.size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                if (ImGui::Selectable(bookmarks[index].name.c_str(),
                    bookmarks[index].name == m_SelectedBookmark))
                {
                    requestedBookmark = index;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        if (requestedBookmark)
        {
            reportAction([&]
            {
                const auto& bookmark = context.scene.CameraBookmarks()[*requestedBookmark];
                // Verify restoration before changing the active authored viewpoint.
                static_cast<void>(CreateSceneCamera(bookmark.camera));
                m_SelectedBookmark = bookmark.name;
                context.scene.SelectCameraBookmark(*requestedBookmark);
                context.cameraRestoreRequest = context.scene.SavedCamera();
            });
        }

        const auto bookmarks = context.scene.CameraBookmarks();
        const auto selected = std::find_if(bookmarks.begin(), bookmarks.end(), [&](const auto& bookmark)
        {
            return bookmark.name == m_SelectedBookmark;
        });
        ImGui::BeginDisabled(selected == bookmarks.end());
        if (ImGui::Button("Update bookmark from current view"))
        {
            reportAction([&]
            {
                context.scene.UpdateCameraBookmark(
                    static_cast<std::size_t>(selected - bookmarks.begin()),
                    CaptureSceneCamera(context.camera, context.referenceViewport));
            });
        }
        if (ImGui::Button("Delete bookmark"))
        {
            reportAction([&]
            {
                context.scene.RemoveCameraBookmark(
                    static_cast<std::size_t>(selected - bookmarks.begin()));
                m_SelectedBookmark.clear();
            });
        }
        ImGui::EndDisabled();
        ImGui::EndDisabled();
        ImGui::TextWrapped("Selecting a bookmark also saves it as the active viewpoint. "
            "Updating a bookmark changes its stored view; select it to make that view active.");
        if (const auto& saved = context.scene.SavedCamera(); saved && saved->referenceViewport)
        {
            const auto extent = *saved->referenceViewport;
            ImGui::Text("Reference viewport: %u x %u (aspect %.4f)",
                extent.width, extent.height,
                static_cast<double>(extent.width) / extent.height);
            if (context.referenceViewport &&
                (context.referenceViewport->width != extent.width ||
                    context.referenceViewport->height != extent.height))
                ImGui::TextWrapped("Current viewport dimensions differ from the reference capture.");
        }

		ImGui::SeparatorText("Rendering");

		ImGui::BeginDisabled(context.cameraEditingDisabled);

		SceneRenderingSettings rendering =
			context.scene.RenderingSettings();

		bool renderingChanged = ImGui::Checkbox(
			"Show environment background",
			&rendering.showEnvironmentBackground);

		ImGui::BeginDisabled(
			rendering.showEnvironmentBackground);

		renderingChanged |= ImGui::ColorEdit3(
			"Fallback background color",
			&rendering.backgroundColor.x,
			ImGuiColorEditFlags_Float);

		ImGui::EndDisabled();

		renderingChanged |= ImGui::SliderFloat(
			"Exposure",
			&rendering.exposure,
			0.05f,
			5.0f,
			"%.2f",
			ImGuiSliderFlags_Logarithmic |
			ImGuiSliderFlags_AlwaysClamp);

		renderingChanged |= ImGui::SliderFloat(
			"Environment intensity",
			&rendering.environmentIntensity,
			0.0f,
			2.0f,
			"%.2f",
			ImGuiSliderFlags_AlwaysClamp);

		ImGui::SeparatorText("Directional shadows");

		renderingChanged |= ImGui::Checkbox(
			"Enable shadows",
			&rendering.directionalShadows.enabled);

		ImGui::BeginDisabled(
			!rendering.directionalShadows.enabled);

		renderingChanged |= ImGui::Checkbox(
			"Enable PCF",
			&rendering.directionalShadows.filteringEnabled);

		renderingChanged |= ImGui::SliderFloat(
			"Minimum bias",
			&rendering.directionalShadows.minimumBias,
			0.0f,
			0.01f,
			"%.6f",
			ImGuiSliderFlags_AlwaysClamp);

		renderingChanged |= ImGui::SliderFloat(
			"Slope bias",
			&rendering.directionalShadows.slopeBias,
			0.0f,
			0.02f,
			"%.6f",
			ImGuiSliderFlags_AlwaysClamp);

		ImGui::EndDisabled();

		if (ImGui::Button("Reset shadow settings"))
		{
			rendering.directionalShadows =
				DirectionalShadowSettings{};
            renderingChanged = true;
		}

		if (renderingChanged)
		{
			if (const auto error =
				context.scene.TrySetRenderingSettings(rendering))
			{
				ImGui::TextWrapped(
					"Cannot update rendering.%.*s: %.*s",
					static_cast<int>(error->property.size()),
					error->property.data(),
					static_cast<int>(error->message.size()),
					error->message.data());
			}
		}


        ImGui::EndDisabled();

        ImGui::SeparatorText("Environment asset");
        ImGui::BeginDisabled(context.cameraEditingDisabled);
        if (!m_EnvironmentInputInitialized)
        {
            std::snprintf(m_EnvironmentAsset.data(), m_EnvironmentAsset.size(), "%s",
                context.scene.RenderingSettings().environmentAsset.c_str());
            m_EnvironmentInputInitialized = true;
        }
        int environmentSource = static_cast<int>(context.scene.RenderingSettings().environmentSource);
        const bool environmentSourceChanged = ImGui::Combo("Source", &environmentSource,
            "Bundled studio\0None\0Project HDR asset\0");
        ImGui::InputText("Project-relative HDR path", m_EnvironmentAsset.data(), m_EnvironmentAsset.size());
        const bool applyEnvironment = ImGui::Button("Apply project HDR");
        if (environmentSourceChanged || applyEnvironment)
        {
            try
            {
                auto settings = context.scene.RenderingSettings();
                settings.environmentSource = applyEnvironment ? SceneEnvironmentSource::ProjectAsset
                    : static_cast<SceneEnvironmentSource>(environmentSource);
                settings.environmentAsset = settings.environmentSource == SceneEnvironmentSource::ProjectAsset
                    ? std::string{ m_EnvironmentAsset.data() } : std::string{};
                context.applyEnvironment(settings);
                m_PresentationMessage = "Environment applied. Save the scene to keep it.";
            }
            catch (const std::exception& error)
            {
                m_PresentationMessage = error.what();
            }
        }
        ImGui::TextWrapped("Example: environments/Cathedral.hdr (relative to the project root).");
        ImGui::EndDisabled();
        if (!m_PresentationMessage.empty())
            ImGui::TextWrapped("%s", m_PresentationMessage.c_str());

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

		ImGui::End();
	}

	void SandboxDebugUI::EndFrame()
	{
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData());
	}

	void SandboxDebugUI::BeginWorkspace()
	{
		if (m_Workspace.Draw())
		{
			m_IsVisible = true;
		}
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