#pragma once

#include "Transform.hpp"
#include "ViewportTransformGizmo.hpp"

namespace ForgeSim::Sandbox
{
	// Returns false if the matrix cannot be represented safely
	// by the existing translation/rotation/scale transform.
	[[nodiscard]] bool TryApplyGizmoTransform(
		Transform& transform,
		const glm::mat4& matrix,
		GizmoOperation operation);
}