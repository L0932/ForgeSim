#include <ForgeSim/Platform/GlfwWindow.hpp>

int main()
{
	ForgeSim::Platform::WindowSpecification windowSpec{
		.width = 1280,
		.height = 720,
		.title = "ForgeSim Sandbox",
		.verticalSync = true
	};

	ForgeSim::Platform::GlfwWindow window(windowSpec);

	window.Show();

	while (!window.ShouldClose())
	{
		window.PollEvents();
		window.SwapBuffers();
	}

	return 0;
}