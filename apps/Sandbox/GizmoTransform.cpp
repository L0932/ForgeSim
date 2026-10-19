#include "GizmoTransform.hpp"

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] bool IsFinite(
			const glm::mat4& matrix) noexcept
		{
			for (int column = 0; column < 4; ++column)
			{
				for (int row = 0; row < 4; ++row)
				{
					if (!std::isfinite(matrix[column][row]))
					{
						return false;
					}
				}
			}

			return true;
		}

		[[nodiscard]] bool MatricesMatch(
			const glm::mat4& left,
			const glm::mat4& right) noexcept
		{
			for (int column = 0; column < 4; ++column)
			{
				for (int row = 0; row < 4; ++row)
				{
					const float tolerance =
						1e-4f * std::max({
							1.0f,
							std::abs(left[column][row]),
							std::abs(right[column][row])
							});

					if (std::abs(
						left[column][row] -
						right[column][row]) > tolerance)
					{
						return false;
					}
				}
			}

			return true;
		}
	}

	bool TryApplyGizmoTransform(
		Transform& transform,
		const glm::mat4& matrix,
		GizmoOperation operation)
	{
		if (!IsFinite(matrix))
		{
			return false;
		}

		Transform candidate = transform;

		switch (operation)
		{
		case GizmoOperation::Translate:
			candidate.position = glm::vec3{ matrix[3] };
			break;

		case GizmoOperation::Rotate:
		{
			glm::mat4 rotation{ 1.0f };

			for (int axis = 0; axis < 3; ++axis)
			{
				const float scale = transform.scale[axis];

				if (!std::isfinite(scale) ||
					std::abs(scale) < 1e-6f)
				{
					return false;
				}

				rotation[axis] = glm::vec4{
					glm::vec3{ matrix[axis] } / scale,
					0.0f
				};
			}

			glm::extractEulerAngleXYZ(
				rotation,
				candidate.rotationRadians.x,
				candidate.rotationRadians.y,
				candidate.rotationRadians.z);

			break;
		}

		case GizmoOperation::Scale:
		{
			const glm::mat4 rotation =
				glm::eulerAngleXYZ(
					transform.rotationRadians.x,
					transform.rotationRadians.y,
					transform.rotationRadians.z);

			for (int axis = 0; axis < 3; ++axis)
			{
				// Project onto the object's known rotation axes.
				// This retains the sign of each scale component.
				candidate.scale[axis] =
					glm::dot(
						glm::vec3{ matrix[axis] },
						glm::vec3{ rotation[axis] });

				if (!std::isfinite(candidate.scale[axis]) ||
					std::abs(candidate.scale[axis]) < 1e-6f)
				{
					return false;
				}
			}

			break;
		}

		default:
			return false;
		}

		const glm::mat4 reconstructed =
			candidate.ModelMatrix();

		if (!IsFinite(reconstructed) ||
			!MatricesMatch(reconstructed, matrix))
		{
			return false;
		}

		transform = candidate;
		return true;
	}
}