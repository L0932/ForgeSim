#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <glad/gl.h>

int main()
{
	try
	{
		ForgeSim::Core::LogInfo("Starting ForgeSim Sandbox Application");

		ForgeSim::Platform::WindowSpecification windowSpec{
			.width = 1280,
			.height = 720,
			.title = "ForgeSim Sandbox",
			.verticalSync = true
		};

		ForgeSim::Platform::GlfwWindow window(windowSpec);

		window.Show();

		glClearColor(0.05f, 0.15f, 0.30f, 1.0f);

		while (!window.ShouldClose())
		{
			window.PollEvents();

			glClear(GL_COLOR_BUFFER_BIT);

			window.SwapBuffers();
		}
	}
	catch (const std::exception& exception)
	{
		ForgeSim::Core::LogError(exception.what());
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS;
}