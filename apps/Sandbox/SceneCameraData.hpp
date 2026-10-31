#pragma once 

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	struct SceneCameraData
	{
		glm::vec3 position{ 0.0f, 0.0f, 5.0f };

		glm::vec3 forwardDirection{ 0.0f, 0.0f, -1.0f };
		glm::vec3 upDirection{ 0.0f, 1.0f, 0.0f };

		float verticalFieldOfViewRadians =
			glm::radians(45.0f);

		float nearClippingPlane = 0.01f;
		float farClippingPlane = 1000.0f;
	};

}