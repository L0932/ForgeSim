#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	struct Transform
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 rotationRadians{ 0.0f };
		glm::vec3 scale{ 1.0f };

		[[nodiscard]] glm::mat4 ModelMatrix() const noexcept;
	};
}