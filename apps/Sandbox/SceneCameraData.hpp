#pragma once 

#include <cstdint>
#include <optional>
#include <string>

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneCaptureExtent
    {
        std::uint32_t width = 0;
        std::uint32_t height = 0;
    };

	struct SceneCameraData
	{
		glm::vec3 position{ 0.0f, 0.0f, 5.0f };

		glm::vec3 forwardDirection{ 0.0f, 0.0f, -1.0f };
		glm::vec3 upDirection{ 0.0f, 1.0f, 0.0f };

		float verticalFieldOfViewRadians =
			glm::radians(45.0f);

		float nearClippingPlane = 0.01f;
		float farClippingPlane = 1000.0f;

        // Physical viewport pixels at the time of explicit capture.
        std::optional<SceneCaptureExtent> referenceViewport{};
	};

    struct SceneCameraBookmark
    {
        std::string name;
        SceneCameraData camera;
    };
}