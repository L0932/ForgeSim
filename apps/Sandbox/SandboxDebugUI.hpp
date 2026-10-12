#pragma once

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

		[[nodiscard]] bool ShowGrid() const noexcept;
		[[nodiscard]] bool ShowObjectBounds() const noexcept;

		[[nodiscard]] bool RuntimeDiagnosticsEnabled()
			const noexcept;

	private:
		bool m_IsVisible = true;
		bool m_ShowGrid = true;
		bool m_ShowObjectBounds = false;
		bool m_RuntimeDiagnosticsEnabled = true;
	};
}