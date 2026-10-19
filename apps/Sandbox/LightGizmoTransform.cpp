#include "LightGizmoTransform.hpp"

#include <cmath>

#include <glm/geometric.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] bool IsFinite(
			const glm::vec3& value) noexcept
		{
			return std::isfinite(value.x) &&
				std::isfinite(value.y) &&
				std::isfinite(value.z);
		}
	}

	bool TryCreateLightGizmoTransform(
		const glm::vec3& position,
		const glm::vec3& emissionDirection,
		Transform& output)
	{
		if (!IsFinite(position) ||
			!IsFinite(emissionDirection))
		{
			return false;
		}

		const float lengthSquared =
			glm::dot(emissionDirection, emissionDirection);

		if (!std::isfinite(lengthSquared) ||
			lengthSquared < 1e-12f)
		{
			return false;
		}

		const glm::vec3 forward =
			emissionDirection / std::sqrt(lengthSquared);

		const glm::vec3 referenceUp =
			std::abs(forward.y) < 0.99f
			? glm::vec3{ 0.0f, 1.0f, 0.0f }
		: glm::vec3{ 1.0f, 0.0f, 0.0f };

		const glm::vec3 right =
			glm::normalize(
				glm::cross(forward, referenceUp));

		const glm::vec3 up =
			glm::cross(right, forward);

		glm::mat4 rotation{ 1.0f };
		rotation[0] = glm::vec4{ right, 0.0f };
		rotation[1] = glm::vec4{ up, 0.0f };
		rotation[2] = glm::vec4{ -forward, 0.0f };

		Transform candidate;
		candidate.position = position;
		candidate.scale = glm::vec3{ 1.0f };

		glm::extractEulerAngleXYZ(
			rotation,
			candidate.rotationRadians.x,
			candidate.rotationRadians.y,
			candidate.rotationRadians.z);

		if (!IsFinite(candidate.rotationRadians))
		{
			return false;
		}

		output = candidate;
		return true;
	}

	glm::vec3 LightGizmoEmissionDirection(
		const Transform& transform)
	{
		return glm::normalize(
			-glm::vec3{ transform.ModelMatrix()[2] });
	}
}