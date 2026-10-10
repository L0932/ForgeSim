#pragma once

#include <optional>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Renderer
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	struct BoundingSphere
	{
		glm::vec3 center;
		float radius;
	};

	struct AxisAlignedBoundingBox
	{
		glm::vec3 minimum;
		glm::vec3 maximum;
	};

	[[nodiscard]] std::optional<float>
		IntersectRayTransformedBox(
			const Ray& ray,
			const AxisAlignedBoundingBox& localBounds,
			const glm::mat4& modelMatrix) noexcept;

	[[nodiscard]] Ray CreateViewportRay(
		double cursorX,
		double cursorY,
		int viewportWidth,
		int viewportHeight,
		const glm::mat4& view,
		const glm::mat4& projection
	);

	[[nodiscard]] std::optional<float> IntersectRaySphere(
		const Ray& ray,
		const BoundingSphere& sphere) noexcept;
}