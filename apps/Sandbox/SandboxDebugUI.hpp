#pragma once

namespace ForgeSim::Platform
{
	class GlfwWindow;
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
		void Draw(SandboxObject* selectedObject);
		void EndFrame();

		[[nodiscard]] bool WantsMouseCapture()
			const noexcept;

		[[nodiscard]] bool WantsKeyboardCapture()
			const noexcept;

	private:
		bool m_IsVisible = true;
	};
}