#include "Transform.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <glm/trigonometric.hpp>
#include <glm/vec4.hpp>

namespace
{
	constexpr float tolerance = 1e-5f;

	[[nodiscard]] bool NearlyEqual(
		float left,
		float right) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}
}

int main()
{
	using ForgeSim::Sandbox::Transform;

	// The default transform should produce an identity matrix.
	{
		const Transform transform;

		const glm::vec4 point =
			transform.ModelMatrix() *
			glm::vec4{ 1.0f, 2.0f, 3.0f, 1.0f };

		if (!NearlyEqual(point.x, 1.0f) ||
			!NearlyEqual(point.y, 2.0f) ||
			!NearlyEqual(point.z, 3.0f) ||
			!NearlyEqual(point.w, 1.0f))
		{
			std::cerr <<
				"Default transform did not produce an identity matrix.\n";

			return EXIT_FAILURE;
		}
	}

	// Translation should move a point by the configured position.
	{
		Transform transform;
		transform.position = glm::vec3{ 2.0f, 3.0f, 4.0f };

		const glm::vec4 point =
			transform.ModelMatrix() *
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(point.x, 2.0f) ||
			!NearlyEqual(point.y, 3.0f) ||
			!NearlyEqual(point.z, 4.0f))
		{
			std::cerr <<
				"Transform translation produced an unexpected point.\n";

			return EXIT_FAILURE;
		}
	}

	// Scale should be applied before translation.
	{
		Transform transform;
		transform.position = glm::vec3{ 3.0f, 0.0f, 0.0f };
		transform.scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		const glm::vec4 point =
			transform.ModelMatrix() *
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(point.x, 5.0f) ||
			!NearlyEqual(point.y, 0.0f) ||
			!NearlyEqual(point.z, 0.0f))
		{
			std::cerr <<
				"Transform scale and translation order was incorrect.\n";

			return EXIT_FAILURE;
		}
	}

	// Positive Y rotation should rotate +X toward -Z.
	{
		Transform transform;

		transform.rotationRadians =
			glm::vec3{
				0.0f,
				glm::radians(90.0f),
				0.0f
		};

		const glm::vec4 point =
			transform.ModelMatrix() *
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(point.x, 0.0f) ||
			!NearlyEqual(point.y, 0.0f) ||
			!NearlyEqual(point.z, -1.0f))
		{
			std::cerr <<
				"Transform Y rotation produced an unexpected point.\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}