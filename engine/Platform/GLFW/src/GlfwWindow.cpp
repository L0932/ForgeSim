#include <ForgeSim/Platform/GlfwWindow.hpp>

#include <optional>
#include <stdexcept>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace ForgeSim::Platform
{
	namespace
	{
		[[nodiscard]] std::optional<Key> TranslateKey(
			int glfwKey) noexcept
		{
			switch (glfwKey)
			{
			case GLFW_KEY_W:
				return Key::W;

			case GLFW_KEY_A:
				return Key::A;

			case GLFW_KEY_S:
				return Key::S;

			case GLFW_KEY_D:
				return Key::D;

			case GLFW_KEY_SPACE:
				return Key::Space;

			case GLFW_KEY_LEFT_CONTROL:
				return Key::LeftControl;

			default:
				return std::nullopt;
			}
		}

		[[nodiscard]] std::optional<MouseButton>
			TranslateMouseButton(int glfwButton) noexcept
		{
			switch (glfwButton)
			{
			case GLFW_MOUSE_BUTTON_RIGHT:
				return MouseButton::Right;

			default:
				return std::nullopt;
			}
		}
	}

	GlfwWindow::GlfwWindow(
		const WindowSpecification& specification
	)
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

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

		glfwSetWindowUserPointer(
			m_Window,
			this);

		glfwSetWindowFocusCallback(
			m_Window,
			[](GLFWwindow* window, int focused)
			{
				auto* owner = static_cast<GlfwWindow*>(
					glfwGetWindowUserPointer(window));

				if (owner != nullptr && focused == GLFW_FALSE)
				{
					owner->m_InputState.Clear();

					glfwSetInputMode(
						window,
						GLFW_CURSOR,
						GLFW_CURSOR_NORMAL);
				}
			});

		glfwSetKeyCallback(
			m_Window,
			[](GLFWwindow* window,
				int key,
				int,
				int action,
				int)
			{
				GlfwWindow* owner =
					static_cast<GlfwWindow*>(
						glfwGetWindowUserPointer(window));

				if (owner == nullptr)
				{
					return;
				}

				const auto translatedKey =
					TranslateKey(key);

				if (!translatedKey)
				{
					return;
				}

				if (action == GLFW_PRESS)
				{
					owner->m_InputState.SetKeyState(
						*translatedKey,
						true);
				}
				else if (action == GLFW_RELEASE)
				{
					owner->m_InputState.SetKeyState(
						*translatedKey,
						false);
				}
			});

		glfwSetMouseButtonCallback(
			m_Window,
			[](GLFWwindow* window,
				int button,
				int action,
				int)
			{
				GlfwWindow* owner =
					static_cast<GlfwWindow*>(
						glfwGetWindowUserPointer(window));

				if (owner == nullptr)
				{
					return;
				}

				const auto translatedButton =
					TranslateMouseButton(button);

				if (!translatedButton)
				{
					return;
				}

				if (action == GLFW_PRESS)
				{
					owner->m_InputState.SetMouseButtonState(
						*translatedButton,
						true);
				}
				else if (action == GLFW_RELEASE)
				{
					owner->m_InputState.SetMouseButtonState(
						*translatedButton,
						false);
				}
			});

		glfwSetCursorPosCallback(
			m_Window,
			[](GLFWwindow* window,
				double x,
				double y)
			{
				GlfwWindow* owner =
					static_cast<GlfwWindow*>(
						glfwGetWindowUserPointer(window));

				if (owner == nullptr)
				{
					return;
				}

				owner->m_InputState.SetCursorPosition(
					CursorPosition{
						.x = x,
						.y = y
					});
			});

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

	const InputState& GlfwWindow::GetInputState()
		const noexcept
	{
		return m_InputState;
	}

	void GlfwWindow::Show()
	{
		glfwShowWindow(m_Window);
	}

	void GlfwWindow::PollEvents()
	{
		m_InputState.BeginFrame();
		glfwPollEvents();
	}

	void GlfwWindow::SetCursorMode(
		CursorMode mode)
	{
		switch (mode)
		{
			case CursorMode::Normal:
				glfwSetInputMode(
					m_Window,
					GLFW_CURSOR,
					GLFW_CURSOR_NORMAL);
				break;

			case CursorMode::Captured:
				glfwSetInputMode(
					m_Window,
					GLFW_CURSOR,
					GLFW_CURSOR_DISABLED);
				break;
		}

		m_InputState.ResetCursorTracking();
	}

	void GlfwWindow::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	bool GlfwWindow::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}

	FramebufferExtent GlfwWindow::GetFramebufferExtent() const
	{
		FramebufferExtent extent;

		glfwGetFramebufferSize(
			m_Window,
			&extent.width,
			&extent.height
		);

		return extent;
	}

	void GlfwWindow::WaitEvents()
	{
		glfwWaitEvents();
	}
}