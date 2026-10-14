#pragma once

#include <glm/vec3.hpp>

namespace ForgeSim::Platform
{
	class GlfwWindow;
}

namespace ForgeSim::Core
{
	struct RuntimeDiagnosticsSnapshot;
}

namespace ForgeSim::Sandbox
{
	struct SandboxObject;

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
		void Draw(
			SandboxObject* selectedObject,
			const Core::RuntimeDiagnosticsSnapshot*
			diagnosticsSnapshot);
		void EndFrame();

		[[nodiscard]] bool WantsMouseCapture()
			const noexcept;

		[[nodiscard]] bool WantsKeyboardCapture()
			const noexcept;

		[[nodiscard]] float Exposure() const noexcept;
		[[nodiscard]] bool ShowGrid() const noexcept;
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
		float m_Exposure = 1.0f;
		bool m_IsVisible = true;
		bool m_ShowGrid = true;
		bool m_ShowObjectBounds = false;
		bool m_RuntimeDiagnosticsEnabled = false;

		bool m_SelectionHighlightEnabled = true;

		glm::vec3 m_SelectionHighlightColor{
			1.0f,
			0.35f,
			0.05f
		};

		float m_SelectionHighlightStrength = 0.20f;
	};
}