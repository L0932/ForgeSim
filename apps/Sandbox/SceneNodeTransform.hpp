#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ForgeSim/Assets/SceneAsset.hpp>

namespace ForgeSim::Sandbox
{
	[[nodiscard]] inline glm::mat4 CreateSceneNodeLocalMatrix(
		const Assets::SceneNodeTransform& transform)
	{
		if (transform.matrix)
		{
			// Both glTF and GLM store matrix components in
			// column-major order.
			return glm::make_mat4(
				transform.matrix->data());
		}

		const glm::vec3 translation{
			transform.translation[0],
			transform.translation[1],
			transform.translation[2]
		};

		// glTF stores quaternion components as X, Y, Z, W,
		// while GLM's constructor accepts W, X, Y, Z.
		const glm::quat rotation{
			transform.rotation[3],
			transform.rotation[0],
			transform.rotation[1],
			transform.rotation[2]
		};

		const glm::vec3 scale{
			transform.scale[0],
			transform.scale[1],
			transform.scale[2]
		};

		return
			glm::translate(
				glm::mat4{ 1.0f },
				translation) *
			glm::mat4_cast(
				glm::normalize(rotation)) *
			glm::scale(
				glm::mat4{ 1.0f },
				scale);
	}
}
