#include "SandboxApplication.hpp"

#include <glad/gl.h>
#include <cassert>

namespace ForgeSim::Sandbox
{
	SandboxApplication::SandboxApplication(const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
	{
		// constructs window
	}

	void SandboxApplication::Run()
	{
		assert(!m_RunAttempted && "Run() can only be called once per application instance");
		m_RunAttempted = true;

		m_Window.Show();

		glClearColor(0.05f, 0.15f, 0.30f, 1.0f);

		while (!m_Window.ShouldClose())
		{
			m_Window.PollEvents();

			glClear(GL_COLOR_BUFFER_BIT);

			m_Window.SwapBuffers();
		}
	}
}