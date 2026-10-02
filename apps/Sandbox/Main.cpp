#include <cstdlib>
#include <exception>

#include <ForgeSim/Core/Log.hpp>
#include "SandboxApplication.hpp"

int main()
{
	try {
		ForgeSim::Core::LogInfo("Starting ForgeSim Sandbox Application");

		ForgeSim::Sandbox::SandboxApplication app {
			ForgeSim::Sandbox::SandboxApplicationSpecification {
				.windowSpec = {
					.width = 1280,
					.height = 720,
					.title = "ForgeSim Sandbox",
					.verticalSync = true
				}
			}
		};

		app.Run();
	}
	catch (const std::exception& e) 
	{
		ForgeSim::Core::LogError(e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}