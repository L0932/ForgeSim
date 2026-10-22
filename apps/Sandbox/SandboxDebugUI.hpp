#pragma once

#include "SceneLight.hpp"
#include "SceneLightValidation.hpp"
#include "SceneSelection.hpp"
#include "TransformEditorUIState.hpp"

#include <optional>
#include <span>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

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

		void BeginFrame();
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

		[[nodiscard]] glm::vec3 BackgroundColor() const noexcept;
		[[nodiscard]] float Exposure() const noexcept;
		[[nodiscard]] float EnvironmentIntensity() const noexcept;

		[[nodiscard]] bool ShowEnvironmentBackground() const noexcept;
		[[nodiscard]] bool ShowGrid() const noexcept;
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
		glm::vec3 m_BackgroundColor{
			0.032f,
			0.036f,
			0.044f
		};

		float m_Exposure = 1.0f;
		float m_EnvironmentIntensity = 0.2f;

		bool m_IsVisible = true;
		bool m_ShowEnvironmentBackground = false;
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
	};
}