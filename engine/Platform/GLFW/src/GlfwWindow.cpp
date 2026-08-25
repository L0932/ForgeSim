#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace ForgeSim::Platform
{
	GlfwWindow::GlfwWindow(
		const WindowSpecification& specification
	)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}
		m_Window = glfwCreateWindow(
						specification.width, 
						specification.height, 
						specification.title.c_str(), 
						nullptr, 
						nullptr);

		if (!m_Window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
		glfwMakeContextCurrent(m_Window);

		const int loadedVersion =
			gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));

		if (loadedVersion == 0)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
			glfwTerminate();
			throw std::runtime_error("Failed to load OpenGL functions");
		}

		if (specification.verticalSync)
		{
			glfwSwapInterval(specification.verticalSync ? 1 : 0); // Enable V-Sync
		}
		else
		{
			glfwSwapInterval(0); // Disable V-Sync
		}

		int framebufferWidth = 0, framebufferHeight = 0;

		glfwGetFramebufferSize(
			m_Window, 
			&framebufferWidth, 
			&framebufferHeight
		);

		glViewport(
			0,
			0,
			framebufferWidth,
			framebufferHeight
		);

		glfwSetFramebufferSizeCallback(
			m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				glViewport(0, 0, width, height);

				if (width > 0 && height > 0)
				{
					glClear(GL_COLOR_BUFFER_BIT);
					glfwSwapBuffers(window);
				}
			}
		);
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

	void GlfwWindow::Show()
	{
		glfwShowWindow(m_Window);
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