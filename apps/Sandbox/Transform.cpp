#include "Transform.hpp"

#include <glm/ext/matrix_transform.hpp>

#include <cmath>

namespace ForgeSim::Sandbox
{
	glm::mat4 Transform::GizmoMatrix() const noexcept
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

    glm::mat4 Transform::ModelMatrix() const noexcept
    {
        return glm::translate(GizmoMatrix(), -pivot);
    }

    bool Transform::TrySetPivot(const glm::vec3& value) noexcept
    {
        for (int axis = 0; axis < 3; ++axis)
            if (!std::isfinite(value[axis])) return false;

        // Move the transform origin to the new pivot without moving geometry.
        const glm::vec3 newPosition{ModelMatrix() * glm::vec4{value, 1.0f}};
        for (int axis = 0; axis < 3; ++axis)
            if (!std::isfinite(newPosition[axis])) return false;
        position = newPosition;
        pivot = value;
        return true;
    }
}
