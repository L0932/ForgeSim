#include <ForgeSim/Core/Application.hpp>

namespace ForgeSim::Core
{
	void Application::Run()
	{
		// Main application loop
		while (true)
		{
			// Update application state
			Update();
			// Render application
			Render();
		}
	}
	void Application::Update()
	{
		// Update application logic here
	}
	void Application::Render()
	{
		// Render application visuals here
	}
}