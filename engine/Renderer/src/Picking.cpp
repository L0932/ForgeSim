#include <ForgeSim/Renderer/Picking.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <glm/vec4.hpp>

namespace ForgeSim::Renderer
{
	std::optional<float> IntersectRayTransformedBox(
		const Ray& ray,
		const AxisAlignedBoundingBox& localBounds,
		const glm::mat4& modelMatrix) noexcept
	{
		constexpr float parallelEpsilon = 1e-6f;

		const float modelDeterminant =
			glm::determinant(modelMatrix);

		if (std::abs(modelDeterminant) <=
			parallelEpsilon)
		{
			return std::nullopt;
		}

		const glm::mat4 inverseModel =
			glm::inverse(modelMatrix);

		const glm::vec3 localOrigin{
			inverseModel *
			glm::vec4{ ray.origin, 1.0f }
		};

		// Do not normalize this direction. Keeping its magnitude
		// preserves the world-ray distance parameter.
		const glm::vec3 localDirection{
			inverseModel *
			glm::vec4{ ray.direction, 0.0f }
		};

		float nearestDistance =
			-std::numeric_limits<float>::infinity();

		float farthestDistance =
			std::numeric_limits<float>::infinity();

		for (glm::length_t axis = 0; axis < 3; ++axis)
		{
			if (std::abs(localDirection[axis]) <=
				parallelEpsilon)
			{
				if (localOrigin[axis] <
					localBounds.minimum[axis] ||
					localOrigin[axis] >
					localBounds.maximum[axis])
				{
					return std::nullopt;
				}

				continue;
			}

			float firstDistance =
				(localBounds.minimum[axis] -
					localOrigin[axis]) /
				localDirection[axis];

			float secondDistance =
				(localBounds.maximum[axis] -
					localOrigin[axis]) /
				localDirection[axis];

			if (firstDistance > secondDistance)
			{
				std::swap(
					firstDistance,
					secondDistance);
			}

			nearestDistance = std::max(
				nearestDistance,
				firstDistance);

			farthestDistance = std::min(
				farthestDistance,
				secondDistance);

			if (nearestDistance > farthestDistance)
			{
				return std::nullopt;
			}
		}

		if (farthestDistance < 0.0f)
		{
			return std::nullopt;
		}

		return nearestDistance >= 0.0f
			? nearestDistance
			: farthestDistance;
	}

	Ray CreateViewportRay(
		double cursorX,
		double cursorY,
		int viewportWidth,
		int viewportHeight,
		const glm::mat4& view,
		const glm::mat4& projection)
	{
		if (viewportWidth <= 0 || viewportHeight <= 0)
		{
			throw std::invalid_argument(
				"Picking viewport dimensions must be positive");
		}

		const float normalizedDeviceX =
			2.0f *
			static_cast<float>(cursorX) /
			static_cast<float>(viewportWidth) -
			1.0f;

		const float normalizedDeviceY =
			1.0f -
			2.0f *
			static_cast<float>(cursorY) /
			static_cast<float>(viewportHeight);

		const glm::mat4 inverseViewProjection =
			glm::inverse(projection * view);

		glm::vec4 nearPoint =
			inverseViewProjection *
			glm::vec4{
				normalizedDeviceX,
				normalizedDeviceY,
				-1.0,
				1.0
		};

		glm::vec4 farPoint =
			inverseViewProjection *
			glm::vec4{
				normalizedDeviceX,
				normalizedDeviceY,
				1.0f,
				1.0f
		};

		constexpr float minimumHomogeneousW =
			std::numeric_limits<float>::epsilon();

		if (std::abs(nearPoint.w) <= minimumHomogeneousW ||
			std::abs(farPoint.w) <= minimumHomogeneousW)
		{
			throw std::runtime_error(
				"Could not convert viewport coordinates "
				"into a world-space ray.");
		}

		nearPoint /= nearPoint.w;
		farPoint /= farPoint.w;

		const glm::mat4 inverseView =
			glm::inverse(view);

		const glm::vec3 origin{
			inverseView[3]
		};

		const glm::vec3 direction =
			glm::vec3{ farPoint - nearPoint };

		const float directionLength =
			glm::length(direction);

		if (directionLength <=
			std::numeric_limits<float>::epsilon())
		{
			throw std::runtime_error(
				"Picking ray direction has zero length.");
		}

		return Ray{
			.origin = origin,
			.direction = direction / directionLength
		};
	}

	std::optional<float> IntersectRaySphere(
		const Ray& ray,
		const BoundingSphere& sphere) noexcept
	{
		if (sphere.radius < 0.0f)
		{
			return std::nullopt;
		}

		const float directionLengthSquared =
			glm::dot(
				ray.direction,
				ray.direction);

		if (directionLengthSquared <=
			std::numeric_limits<float>::epsilon())
		{
			return std::nullopt;
		}

		const glm::vec3 originToCenter =
			ray.origin - sphere.center;

		const float halfB =
			glm::dot(
				originToCenter,
				ray.direction);

		const float c =
			glm::dot(
				originToCenter,
				originToCenter) -
			sphere.radius * sphere.radius;

		const float discriminant =
			halfB * halfB -
			directionLengthSquared * c;

		if (discriminant < 0.0f)
		{
			return std::nullopt;
		}

		const float squareRoot =
			std::sqrt(discriminant);

		float distance =
			(-halfB - squareRoot) /
			directionLengthSquared;

		if (distance >= 0.0f)
		{
			return distance;
		}

		distance =
			(-halfB + squareRoot) /
			directionLengthSquared;

		if (distance >= 0.0f)
		{
			return distance;
		}

		return std::nullopt;
	}
}