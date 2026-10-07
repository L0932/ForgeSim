#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace ForgeSim::Renderer
{
	PerspectiveCamera::PerspectiveCamera(
		const glm::vec3& position,
		const glm::vec3& target,
		const glm::vec3& up,
		float verticalFieldOfViewRadians,
		float nearClippingPlane,
		float farClippingPlane)
		: m_Position(position)
		, m_Target(target)
		, m_Up(up)
		, m_VerticalFieldOfViewRadians(
			verticalFieldOfViewRadians)
		, m_NearClippingPlane(nearClippingPlane)
		, m_FarClippingPlane(farClippingPlane)
	{
		if (verticalFieldOfViewRadians <= 0.0f)
		{
			throw std::invalid_argument(
				"Camera field of view must be positive.");
		}

		if (nearClippingPlane <= 0.0f)
		{
			throw std::invalid_argument(
				"Camera near clipping plane must be positive.");
		}

		if (farClippingPlane <= nearClippingPlane)
		{
			throw std::invalid_argument(
				"Camera far clipping plane must be greater "
				"than its near clipping plane.");
		}
	}

	glm::mat4 PerspectiveCamera::ViewMatrix() const
	{
		return glm::lookAt(
			m_Position,
			m_Target,
			m_Up);
	}

	glm::mat4 PerspectiveCamera::ProjectionMatrix(
		float aspectRatio) const
	{
		if (aspectRatio <= 0.0f)
		{
			throw std::invalid_argument(
				"Camera aspect ratio must be positive.");
		}

		return glm::perspective(
			m_VerticalFieldOfViewRadians,
			aspectRatio,
			m_NearClippingPlane,
			m_FarClippingPlane);
	}
}