#include "SandboxApplication.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>

#include <ForgeSim/Core/Log.hpp>

int main(
	int argumentCount,
	char* arguments[])
{
	try {
		ForgeSim::Core::LogInfo("Starting ForgeSim Sandbox Application");

		const std::filesystem::path sceneFilename =
			argumentCount > 1
			? std::filesystem::path{ arguments[1] }
			: std::filesystem::path{
				"PbrMaterialGallery.glb"
		};

		ForgeSim::Sandbox::SandboxApplication app {
			ForgeSim::Sandbox::SandboxApplicationSpecification {
				.windowSpec = {
					.width = 1280,
					.height = 720,
					.title = "ForgeSim Sandbox",
					.verticalSync = true
				},
				.rendererSpec = {
					.sceneFilename = sceneFilename
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