#pragma once

#include "DirectionalShadowSettings.hpp"
#include "EditorWorkspace.hpp"
#include "SceneLight.hpp"
#include "SceneLightValidation.hpp"
#include "SceneSelection.hpp"
#include "TransformEditorUIState.hpp"

#include "SceneCameraData.hpp"
#include "SceneRenderingSettings.hpp"

#include <array>
#include <functional>
#include <string>
#include <optional>
#include <span>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Renderer/FxaaSettings.hpp>

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

		SandboxScene& scene;
		bool cameraEditingDisabled = false;
        std::optional<SceneCaptureExtent> referenceViewport;
        std::optional<SceneCameraData>& cameraRestoreRequest;
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

		void DrawLoadingFrame(const Assets::GltfLoadProgress& progress, int width, int height);
		void BeginFrame();
        void ResetSceneControls();
		void Draw(const SandboxDebugUIContext& context);
		void EndFrame();

		void BeginWorkspace();

		[[nodiscard]] TransformEditorUIState DrawTransformEditorControls(
			bool editingActive,
			bool navigationActive);

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
	};
}