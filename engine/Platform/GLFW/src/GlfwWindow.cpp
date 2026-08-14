#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace ForgeSim::Platform
{
	GlfwWindow::GlfwWindow(
		const WindowSpecification& specification
	)
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}
		m_Window = glfwCreateWindow(specification.width, specification.height, specification.title.c_str(), nullptr, nullptr);
		if (!m_Window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
		glfwMakeContextCurrent(m_Window);
		if (specification.verticalSync)
		{
			glfwSwapInterval(1); // Enable V-Sync
		}
		else
		{
			glfwSwapInterval(0); // Disable V-Sync
		}
	}
	GlfwWindow::~GlfwWindow()
	{
		if (m_Window)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
			glfwTerminate();
		}
	}
	void GlfwWindow::PollEvents()
	{
		glfwPollEvents();
	}
	void GlfwWindow::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}
	bool GlfwWindow::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}
}