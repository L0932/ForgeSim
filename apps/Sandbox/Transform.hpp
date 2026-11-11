#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	struct Transform
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 rotationRadians{ 0.0f };
		glm::vec3 scale{ 1.0f };

        // Asset-space point placed at position; zero preserves the authored origin.
        glm::vec3 pivot{ 0.0f };

        [[nodiscard]] glm::mat4 GizmoMatrix() const noexcept;
        [[nodiscard]] bool TrySetPivot(const glm::vec3& value) noexcept;

		[[nodiscard]] glm::mat4 ModelMatrix() const noexcept;
	};
}
