#include "LightGizmoTransform.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>

namespace
{
	using namespace ForgeSim::Sandbox;

	void Require(bool condition, const char* message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	bool NearlyEqual(
		const glm::vec3& left,
		const glm::vec3& right)
	{
		for (int axis = 0; axis < 3; ++axis)
		{
			if (!std::isfinite(left[axis]) ||
				!std::isfinite(right[axis]) ||
				std::abs(left[axis] - right[axis]) > 1e-4f)
			{
				return false;
			}
		}

		return true;
	}

	void TestDirectionRoundTrips()
	{
		const std::array<glm::vec3, 10> directions{
			glm::vec3{ 1.0f, 0.0f, 0.0f },
			glm::vec3{ -1.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, -1.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 1.0f },
			glm::vec3{ 0.0f, 0.0f, -1.0f },
			glm::vec3{ 1.0f, -2.0f, 3.0f },
			glm::vec3{ 0.001f, 1.0f, 0.001f },
			glm::vec3{ 0.001f, -1.0f, 0.001f },
			glm::vec3{ 0.2f, 0.99f, -0.1f }
		};

		const glm::vec3 position{ 2.0f, -3.0f, 4.0f };

		for (const glm::vec3& direction : directions)
		{
			Transform transform;

			Require(
				TryCreateLightGizmoTransform(
					position,
					direction,
					transform),
				"Valid light direction was rejected.");

			Require(
				NearlyEqual(transform.position, position),
				"Conversion changed the light position.");

			Require(
				NearlyEqual(
					transform.scale,
					glm::vec3{ 1.0f }),
				"Light gizmo must have unit scale.");

			const glm::vec3 expected =
				glm::normalize(direction);

			// Check the actual matrix convention independently
			// of the extraction helper.
			const glm::vec3 forward =
				-glm::vec3{ transform.ModelMatrix()[2] };

			Require(
				NearlyEqual(forward, expected),
				"Local -Z does not follow emission.");

			Require(
				NearlyEqual(
					LightGizmoEmissionDirection(transform),
					expected),
				"Emission direction did not round-trip.");
		}
	}

	void RequireRejected(
		const glm::vec3& position,
		const glm::vec3& direction)
	{
		Transform output;
		output.position = { 1.0f, 2.0f, 3.0f };
		output.rotationRadians = { 0.2f, 0.3f, 0.4f };
		output.scale = { 2.0f, 3.0f, 4.0f };

		const Transform before = output;

		Require(
			!TryCreateLightGizmoTransform(
				position,
				direction,
				output),
			"Invalid input was accepted.");

		Require(
			output.position == before.position &&
			output.rotationRadians == before.rotationRadians &&
			output.scale == before.scale,
			"Rejected conversion changed the output.");
	}

	void TestInvalidInputs()
	{
		const float nan =
			std::numeric_limits<float>::quiet_NaN();

		const float infinity =
			std::numeric_limits<float>::infinity();

		RequireRejected(
			glm::vec3{ 0.0f },
			glm::vec3{ 0.0f });

		RequireRejected(
			glm::vec3{ 0.0f },
			glm::vec3{ 1e-8f, 0.0f, 0.0f });

		RequireRejected(
			glm::vec3{ 0.0f },
			glm::vec3{ nan, 1.0f, 0.0f });

		RequireRejected(
			glm::vec3{ 0.0f },
			glm::vec3{ 0.0f, infinity, 0.0f });

		RequireRejected(
			glm::vec3{ nan, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, -1.0f });

		RequireRejected(
			glm::vec3{ 0.0f, infinity, 0.0f },
			glm::vec3{ 0.0f, 0.0f, -1.0f });
	}
}

int main()
{
	try
	{
		TestDirectionRoundTrips();
		TestInvalidInputs();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}