#include <ForgeSim/Platform/GlfwWindow.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
	using ForgeSim::Platform::FramebufferExtent;

	if (!FramebufferExtent{ 1280, 720 }.IsDrawable())
	{
		std::cerr << "A positive framebuffer extent should be drawable\n";
		return EXIT_FAILURE;
	}

	if (FramebufferExtent{ 0, 720 }.IsDrawable())
	{
		std::cerr << "A zero-width framebuffer should not be drawable\n";
		return EXIT_FAILURE;
	}

	if (FramebufferExtent{ 1280, 0 }.IsDrawable())
	{
		std::cerr << "A zero-height framebuffer should not be drawable\n";
		return EXIT_FAILURE;
	}

	if (FramebufferExtent{ 0, 0 }.IsDrawable())
	{
		std::cerr << "A zero-sized framebuffer should not be drawable\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}