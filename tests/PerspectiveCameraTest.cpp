#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace
{
	constexpr float tolerance = 1e-5f;

	[[nodiscard]] bool NearlyEqual(
		float left,
		float right)
	{
		return std::abs(left - right) <= tolerance;
	}
}

int main()
{
	using ForgeSim::Renderer::PerspectiveCamera;

	const PerspectiveCamera camera{
		glm::vec3{ 0.0f, 0.0f, 3.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f },
		glm::radians(45.0f),
		0.1f,
		100.0f
	};

	// The world origin should be three units in front of the camera.
	{
		const glm::vec4 viewSpaceOrigin =
			camera.ViewMatrix() *
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(viewSpaceOrigin.x, 0.0f) ||
			!NearlyEqual(viewSpaceOrigin.y, 0.0f) ||
			!NearlyEqual(viewSpaceOrigin.z, -3.0f) ||
			!NearlyEqual(viewSpaceOrigin.w, 1.0f))
		{
			std::cerr <<
				"View matrix did not transform the world origin "
				"to the expected camera-space position.\n";

			return EXIT_FAILURE;
		}
	}

	// A wider aspect ratio should reduce horizontal projection scale
	// without changing vertical projection scale.
	{
		const glm::mat4 squareProjection =
			camera.ProjectionMatrix(1.0f);

		const glm::mat4 wideProjection =
			camera.ProjectionMatrix(2.0f);

		if (!NearlyEqual(
			wideProjection[0][0],
			squareProjection[0][0] / 2.0f))
		{
			std::cerr <<
				"Projection did not account for aspect ratio.\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			wideProjection[1][1],
			squareProjection[1][1]))
		{
			std::cerr <<
				"Vertical projection scale changed unexpectedly.\n";

			return EXIT_FAILURE;
		}
	}

	// Zero aspect ratios must be rejected.
	{
		bool rejectedZeroAspectRatio = false;

		try
		{
			[[maybe_unused]] const glm::mat4 projection =
				camera.ProjectionMatrix(0.0f);
		}
		catch (const std::invalid_argument&)
		{
			rejectedZeroAspectRatio = true;
		}

		if (!rejectedZeroAspectRatio)
		{
			std::cerr <<
				"Camera accepted a zero aspect ratio.\n";

			return EXIT_FAILURE;
		}
	}

	// Translation should move the camera while preserving its orientation.
	{
		PerspectiveCamera translatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		const glm::vec3 originalForward =
			translatedCamera.ForwardDirection();

		translatedCamera.Translate(
			glm::vec3{ 0.0f, 0.0f, -1.0f });

		const glm::vec4 viewSpaceOrigin =
			translatedCamera.ViewMatrix() *
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(viewSpaceOrigin.z, -2.0f))
		{
			std::cerr <<
				"Camera translation produced an unexpected position.\n";

			return EXIT_FAILURE;
		}

		const glm::vec3 translatedForward =
			translatedCamera.ForwardDirection();

		if (!NearlyEqual(translatedForward.x, originalForward.x) ||
			!NearlyEqual(translatedForward.y, originalForward.y) ||
			!NearlyEqual(translatedForward.z, originalForward.z))
		{
			std::cerr <<
				"Camera translation changed its orientation.\n";

			return EXIT_FAILURE;
		}
	}

	// Yaw rotation should rotate the forward direction around the up axis.
	{
		PerspectiveCamera rotatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		rotatedCamera.Rotate(
			glm::radians(90.0f),
			0.0f);

		const glm::vec3 forward =
			rotatedCamera.ForwardDirection();

		if (!NearlyEqual(forward.x, -1.0f) ||
			!NearlyEqual(forward.y, 0.0f) ||
			!NearlyEqual(forward.z, 0.0f))
		{
			std::cerr <<
				"Camera yaw produced an unexpected direction.\n";

			return EXIT_FAILURE;
		}
	}

	// Pitch rotation should remain within the configured safe range.
	{
		PerspectiveCamera rotatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		rotatedCamera.Rotate(
			0.0f,
			glm::radians(180.0f));

		const glm::vec3 forward =
			rotatedCamera.ForwardDirection();

		const float maximumVerticalComponent =
			std::sin(glm::radians(89.0f));

		if (forward.y >
			maximumVerticalComponent + tolerance)
		{
			std::cerr <<
				"Camera pitch exceeded its safe range.\n";

			return EXIT_FAILURE;
		}

		if (forward.y < 0.99f)
		{
			std::cerr <<
				"Camera pitch was not applied up to the clamp.\n";

			return EXIT_FAILURE;
		}
	}

	// Translation should move the camera while preserving its orientation.
	{
		PerspectiveCamera translatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		const glm::vec3 originalForward =
			translatedCamera.ForwardDirection();

		translatedCamera.Translate(
			glm::vec3{ 0.0f, 0.0f, -1.0f });

		const glm::vec4 viewSpaceOrigin =
			translatedCamera.ViewMatrix() *
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		if (!NearlyEqual(viewSpaceOrigin.z, -2.0f))
		{
			std::cerr <<
				"Camera translation produced an unexpected position.\n";

			return EXIT_FAILURE;
		}

		const glm::vec3 translatedForward =
			translatedCamera.ForwardDirection();

		if (!NearlyEqual(translatedForward.x, originalForward.x) ||
			!NearlyEqual(translatedForward.y, originalForward.y) ||
			!NearlyEqual(translatedForward.z, originalForward.z))
		{
			std::cerr <<
				"Camera translation changed its orientation.\n";

			return EXIT_FAILURE;
		}
	}

	// Yaw rotation should rotate the forward direction around the up axis.
	{
		PerspectiveCamera rotatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		rotatedCamera.Rotate(
			glm::radians(90.0f),
			0.0f);

		const glm::vec3 forward =
			rotatedCamera.ForwardDirection();

		if (!NearlyEqual(forward.x, -1.0f) ||
			!NearlyEqual(forward.y, 0.0f) ||
			!NearlyEqual(forward.z, 0.0f))
		{
			std::cerr <<
				"Camera yaw produced an unexpected direction.\n";

			return EXIT_FAILURE;
		}
	}

	// Pitch rotation should remain within the configured safe range.
	{
		PerspectiveCamera rotatedCamera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		rotatedCamera.Rotate(
			0.0f,
			glm::radians(180.0f));

		const glm::vec3 forward =
			rotatedCamera.ForwardDirection();

		const float maximumVerticalComponent =
			std::sin(glm::radians(89.0f));

		if (forward.y >
			maximumVerticalComponent + tolerance)
		{
			std::cerr <<
				"Camera pitch exceeded its safe range.\n";

			return EXIT_FAILURE;
		}

		if (forward.y < 0.99f)
		{
			std::cerr <<
				"Camera pitch was not applied up to the clamp.\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}