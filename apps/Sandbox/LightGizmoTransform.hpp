#pragma once

#include "Transform.hpp"

namespace ForgeSim::Sandbox
{
	// Builds a unit-scale transform whose local -Z axis
	// follows emissionDirection. Failure leaves output unchanged.
	[[nodiscard]] bool TryCreateLightGizmoTransform(
		const glm::vec3& position,
		const glm::vec3& emissionDirection,
		Transform& output);

	// Requires a valid light gizmo transform with unit scale.
	[[nodiscard]] glm::vec3 LightGizmoEmissionDirection(
		const Transform& transform);
}