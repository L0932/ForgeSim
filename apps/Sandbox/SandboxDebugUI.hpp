#pragma once

#include "DirectionalShadowSettings.hpp"
#include "EditorWorkspace.hpp"
#include "SceneLight.hpp"
#include "SceneLightValidation.hpp"
#include "SceneSelection.hpp"
#include "TransformEditorUIState.hpp"
#include "SceneViewportToolbar.hpp"
#include "SceneViewportPanel.hpp"

#include "SceneCameraData.hpp"
#include "FreeCameraController.hpp"
#include "SceneRenderingSettings.hpp"

#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <span>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Renderer/FxaaSettings.hpp>
#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>

namespace ForgeSim::Platform
{
	class GlfwWindow;
}

namespace ForgeSim::Core
{
	struct RuntimeDiagnosticsSnapshot;
}

namespace ForgeSim::Renderer
{
	class PerspectiveCamera;
}

namespace ForgeSim::Renderer
{
	class PerspectiveCamera;
	struct AxisAlignedBoundingBox;
}

namespace ForgeSim::Sandbox
{
	class SandboxScene;

	struct DirectionalLight;
	struct SandboxObject;

	struct SandboxDebugUIContext
	{
		const Core::RuntimeDiagnosticsSnapshot*
			diagnosticsSnapshot = nullptr;

		const Renderer::PerspectiveCamera& camera;
        FreeCameraController& cameraController;

		SandboxScene& scene;
		bool cameraEditingDisabled = false;
        std::optional<SceneCaptureExtent> referenceViewport;
        std::optional<SceneCameraData>& cameraRestoreRequest;
        bool canFocusSelection = false;
        std::function<void()> focusSelection;
        bool orbitActive = false;
        std::function<void()> toggleOrbit;
        std::function<void()> frameScene;
        std::function<void(const SceneRenderingSettings&)> applyEnvironment;
	};

	class SandboxDebugUI final
	{
	public:
		explicit SandboxDebugUI(
			Platform::GlfwWindow& window);

		~SandboxDebugUI();

		SandboxDebugUI(
			const SandboxDebugUI&) = delete;

		SandboxDebugUI(
			SandboxDebugUI&&) = delete;

		SandboxDebugUI& operator=(
			SandboxDebugUI&&) = delete;

		bool DrawLoadingFrame(const Assets::GltfLoadProgress& progress, int width, int height,
			std::optional<std::string_view> error = std::nullopt);
		void BeginFrame();
        void ResetSceneControls();
		void Draw(const SandboxDebugUIContext& context);
		void EndFrame();

		[[nodiscard]] bool BeginWorkspace(const std::function<void()>& drawMainToolbar = {});

		[[nodiscard]] TransformEditorUIState UpdateTransformEditorShortcuts(
			bool editingActive,
			bool navigationActive);

        // Draw over the Scene image and suppress input under its tools overlay.
        [[nodiscard]] TransformEditorUIState DrawTransformEditorToolbar(
            SceneViewportState& viewport, bool disabled);

		[[nodiscard]] bool WantsMouseCapture()
			const noexcept;

		[[nodiscard]] bool WantsKeyboardCapture()
			const noexcept;

		[[nodiscard]] bool ShowGrid() const noexcept;
        [[nodiscard]] const Renderer::FxaaSettings& Fxaa() const noexcept { return m_FxaaSettings; }
		[[nodiscard]] bool ShowLights() const noexcept;

		[[nodiscard]] bool ShowObjectBounds() const noexcept;

		[[nodiscard]] bool SelectionHighlightEnabled()
			const noexcept;
		[[nodiscard]] const glm::vec3& SelectionHighlightColor()
			const noexcept;
		[[nodiscard]] float SelectionHighlightStrength()
			const noexcept;
		[[nodiscard]] bool RuntimeDiagnosticsEnabled()
			const noexcept;


	private:
		std::optional<Renderer::OpenGL::Texture2D> m_LoadingIcon;
		bool m_LoadingIconAttempted = false;
        std::array<char, 129> m_BookmarkName{};
        std::string m_SelectedBookmark;
        std::array<char, 4096> m_EnvironmentAsset{};
        bool m_EnvironmentInputInitialized = false;
        std::string m_PresentationMessage;

		bool m_IsVisible = true;
		Renderer::FxaaSettings m_FxaaSettings{};
		bool m_ShowGrid = true;
		bool m_ShowLights = true;
		bool m_ShowObjectBounds = false;
		bool m_RuntimeDiagnosticsEnabled = false;
		bool m_SelectionHighlightEnabled = true;

		glm::vec3 m_SelectionHighlightColor{
			1.0f,
			0.35f,
			0.05f
		};

		float m_SelectionHighlightStrength = 0.20f;

		TransformEditorUIState m_TransformEditorState;

		EditorWorkspace m_Workspace;
        SceneViewportTools m_ViewportTools;
	};
}