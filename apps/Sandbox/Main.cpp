#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <glad/gl.h>

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
	try
	{
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
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS;
}