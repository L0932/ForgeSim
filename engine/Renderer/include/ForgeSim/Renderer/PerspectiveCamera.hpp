#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <glm/geometric.hpp>

namespace ForgeSim::Renderer
{
	class PerspectiveCamera final
	{
	public:
		PerspectiveCamera(
			const glm::vec3& position,
			const glm::vec3& target,
			const glm::vec3& up,
			float verticalFiedOfViewRadians,
			float nearClippingPlane,
			float farClippingPlane
		);

		[[nodiscard]] glm::mat4 ViewMatrix() const;

		[[nodiscard]] glm::mat4 ProjectionMatrix(
			float aspectRatio) const;

		[[nodiscard]] glm::vec3 ForwardDirection() const;
		[[nodiscard]] glm::vec3 RightDirection() const;

		void Translate(
			const glm::vec3& displacement) noexcept;

		void Rotate(
			float yawDeltaRadians,
			float pitchDeltaRadians) noexcept;

	private:
		glm::vec3 m_Position;
		glm::vec3 m_Target;
		glm::vec3 m_Up;

		float m_VerticalFieldOfViewRadians;
		float m_NearClippingPlane;
		float m_FarClippingPlane;
	};
}