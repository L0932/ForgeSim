#include "Transform.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace ForgeSim::Sandbox
{
	glm::mat4 Transform::ModelMatrix() const noexcept
	{
		glm::mat4 model{ 1.0f };

		model = glm::translate(
			model,
			position);

		model = glm::rotate(
			model,
			rotationRadians.x,
			glm::vec3{ 1.0f, 0.0f, 0.0f });

		model = glm::rotate(
			model,
			rotationRadians.y,
			glm::vec3{ 0.0f, 1.0f, 0.0f });

		model = glm::rotate(
			model,
			rotationRadians.z,
			glm::vec3{ 0.0f, 0.0f, 1.0f });

		model = glm::scale(
			model,
			scale);

		return model;
	}
}