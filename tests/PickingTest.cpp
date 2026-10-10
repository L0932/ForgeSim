#include <ForgeSim/Renderer/PerspectiveCamera.hpp>
#include <ForgeSim/Renderer/Picking.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace
{
	[[nodiscard]] bool NearlyEqual(
		float left,
		float right,
		float tolerance = 1e-5f) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}

	[[nodiscard]] bool NearlyEqual(
		const glm::vec3& left,
		const glm::vec3& right,
		float tolerance = 1e-5f) noexcept
	{
		return
			NearlyEqual(left.x, right.x, tolerance) &&
			NearlyEqual(left.y, right.y, tolerance) &&
			NearlyEqual(left.z, right.z, tolerance);
	}
}

int main()
{
	using ForgeSim::Renderer::BoundingSphere;
	using ForgeSim::Renderer::CreateViewportRay;
	using ForgeSim::Renderer::IntersectRaySphere;
	using ForgeSim::Renderer::PerspectiveCamera;
	using ForgeSim::Renderer::Ray;

	// The center of the viewport should produce the camera's
	// forward direction.
	{
		const PerspectiveCamera camera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		const Ray ray = CreateViewportRay(
			400.0,
			300.0,
			800,
			600,
			camera.ViewMatrix(),
			camera.ProjectionMatrix(800.0f / 600.0f));

		if (!NearlyEqual(
			ray.origin,
			glm::vec3{ 0.0f, 0.0f, 3.0f }))
		{
			std::cerr
				<< "Center ray had an unexpected origin.\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			ray.direction,
			glm::vec3{ 0.0f, 0.0f, -1.0f }))
		{
			std::cerr
				<< "Center ray did not follow "
				<< "the camera's forward direction.\n";

			return EXIT_FAILURE;
		}
	}

	// A forward ray should intersect the sphere's nearest surface.
	{
		const Ray ray{
			.origin = glm::vec3{ 0.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, -1.0f }
		};

		const BoundingSphere sphere{
			.center = glm::vec3{ 0.0f },
			.radius = 1.0f
		};

		const auto distance =
			IntersectRaySphere(ray, sphere);

		if (!distance || !NearlyEqual(*distance, 2.0f))
		{
			std::cerr
				<< "Ray did not return the nearest "
				<< "sphere intersection.\n";

			return EXIT_FAILURE;
		}
	}

	// A ray directed away from the sphere should miss it.
	{
		const Ray ray{
			.origin = glm::vec3{ 0.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, 1.0f }
		};

		const BoundingSphere sphere{
			.center = glm::vec3{ 0.0f },
			.radius = 1.0f
		};

		if (IntersectRaySphere(ray, sphere))
		{
			std::cerr
				<< "A ray pointing away from the sphere "
				<< "reported an intersection.\n";

			return EXIT_FAILURE;
		}
	}

	// A ray beginning inside a sphere should return the exit point.
	{
		const Ray ray{
			.origin = glm::vec3{ 0.0f },
			.direction = glm::vec3{ 1.0f, 0.0f, 0.0f }
		};

		const BoundingSphere sphere{
			.center = glm::vec3{ 0.0f },
			.radius = 2.0f
		};

		const auto distance =
			IntersectRaySphere(ray, sphere);

		if (!distance || !NearlyEqual(*distance, 2.0f))
		{
			std::cerr
				<< "Ray inside a sphere did not return "
				<< "the positive exit distance.\n";

			return EXIT_FAILURE;
		}
	}

	// Invalid viewport dimensions should be rejected.
	{
		bool exceptionWasThrown = false;

		try
		{
			static_cast<void>(
				CreateViewportRay(
					0.0,
					0.0,
					0,
					600,
					glm::mat4{ 1.0f },
					glm::mat4{ 1.0f }));
		}
		catch (const std::invalid_argument&)
		{
			exceptionWasThrown = true;
		}

		if (!exceptionWasThrown)
		{
			std::cerr
				<< "Invalid viewport dimensions "
				<< "were not rejected.\n";

			return EXIT_FAILURE;
		}
	}

	const ForgeSim::Renderer::AxisAlignedBoundingBox unitBox{
		.minimum = glm::vec3{ -0.5f },
		.maximum = glm::vec3{ 0.5f }
	};

	// A centered ray should hit an untransformed unit box.
	{
		const ForgeSim::Renderer::Ray ray{
			.origin = glm::vec3{ 0.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, -1.0f }
		};

		const auto distance =
			ForgeSim::Renderer::IntersectRayTransformedBox(
				ray,
				unitBox,
				glm::mat4{ 1.0f });

		if (!distance ||
			!NearlyEqual(*distance, 2.5f))
		{
			std::cerr
				<< "Expected the ray to hit the unit box\n";

			return EXIT_FAILURE;
		}
	}

	// A ray outside the box should miss.
	{
		const ForgeSim::Renderer::Ray ray{
			.origin = glm::vec3{ 2.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, -1.0f }
		};

		const auto distance =
			ForgeSim::Renderer::IntersectRayTransformedBox(
				ray,
				unitBox,
				glm::mat4{ 1.0f });

		if (distance)
		{
			std::cerr
				<< "Expected the ray to miss the unit box\n";

			return EXIT_FAILURE;
		}
	}

	// When two boxes overlap along a ray, the front hit must be closer.
	{
		const ForgeSim::Renderer::Ray ray{
			.origin = glm::vec3{ 0.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, -1.0f }
		};

		const glm::mat4 frontModel{ 1.0f };

		const glm::mat4 rearModel =
			glm::translate(
				glm::mat4{ 1.0f },
				glm::vec3{ 0.0f, 0.0f, -3.0f });

		const auto frontDistance =
			ForgeSim::Renderer::IntersectRayTransformedBox(
				ray,
				unitBox,
				frontModel);

		const auto rearDistance =
			ForgeSim::Renderer::IntersectRayTransformedBox(
				ray,
				unitBox,
				rearModel);

		if (!frontDistance ||
			!rearDistance ||
			*frontDistance >= *rearDistance)
		{
			std::cerr
				<< "Expected the front box to be hit first\n";

			return EXIT_FAILURE;
		}
	}

	// A non-invertible transform cannot be intersected.
	{
		const glm::mat4 zeroScaleModel =
			glm::scale(
				glm::mat4{ 1.0f },
				glm::vec3{ 0.0f });

		const ForgeSim::Renderer::Ray ray{
			.origin = glm::vec3{ 0.0f, 0.0f, 3.0f },
			.direction = glm::vec3{ 0.0f, 0.0f, -1.0f }
		};

		if (ForgeSim::Renderer::IntersectRayTransformedBox(
			ray,
			unitBox,
			zeroScaleModel))
		{
			std::cerr
				<< "Expected a non-invertible box transform "
				<< "to be rejected\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}