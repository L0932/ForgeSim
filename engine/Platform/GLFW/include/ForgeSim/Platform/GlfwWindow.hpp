#pragma once

#include <string>

struct GLFWwindow;

namespace ForgeSim::Platform
{
	struct WindowSpecification {
		int width;
		int height;
		std::string title;
		bool verticalSync = true;
	};

	struct FramebufferExtent {
		int width = 0;
		int height = 0;

		[[nodiscard]] constexpr bool IsDrawable() const noexcept
		{
			return width > 0 && height > 0;
		}
	};

	class GlfwWindow
	{
	public:
		explicit GlfwWindow(const
			WindowSpecification& specification = {}
		);

		~GlfwWindow();
		
		GlfwWindow(const GlfwWindow&) = delete;
		GlfwWindow& operator=(const GlfwWindow&) = delete;

		GlfwWindow(GlfwWindow&&) = delete;
		GlfwWindow& operator=(GlfwWindow&&) = delete;

		void Show();
		void PollEvents();
		void SwapBuffers();

		[[nodiscard]] bool ShouldClose() const;
		[[nodiscard]] FramebufferExtent GetFramebufferExtent() const;
		void WaitEvents();
		
	private:
		GLFWwindow* m_Window = nullptr;
	};
}