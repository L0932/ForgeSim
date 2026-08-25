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
		
	private:
		GLFWwindow* m_Window = nullptr;
	};
}