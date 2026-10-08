#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec4.hpp>

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

	void PerspectiveCamera::Translate(
		const glm::vec3& displacement) noexcept
	{
		m_Position += displacement;
		m_Target += displacement;
	}

	glm::vec3 PerspectiveCamera::ForwardDirection() const
	{
		return glm::normalize(
			m_Target - m_Position);
	}

	glm::vec3 PerspectiveCamera::RightDirection() const
	{
		return glm::normalize(
			glm::cross(
				ForwardDirection(),
				m_Up));
	}

	void PerspectiveCamera::Rotate(
		float yawDeltaRadians,
		float pitchDeltaRadians) noexcept
	{
		const glm::vec3 up =
			glm::normalize(m_Up);

		glm::vec3 forward =
			ForwardDirection();

		const float currentPitch = std::asin(
			std::clamp(
				glm::dot(forward, up),
				-1.0f,
				1.0f));

		const float maximumPitch =
			glm::radians(89.0f);

		const float targetPitch = std::clamp(
			currentPitch + pitchDeltaRadians,
			-maximumPitch,
			maximumPitch);

		const float appliedPitch =
			targetPitch - currentPitch;

		const glm::mat4 yawRotation = glm::rotate(
			glm::mat4{ 1.0f },
			yawDeltaRadians,
			up);

		forward = glm::normalize(
			glm::vec3{
				yawRotation *
				glm::vec4{ forward, 0.0f }
			});

		const glm::vec3 right = glm::normalize(
			glm::cross(forward, up));

		const glm::mat4 pitchRotation = glm::rotate(
			glm::mat4{ 1.0f },
			appliedPitch,
			right);

		forward = glm::normalize(
			glm::vec3{
				pitchRotation *
				glm::vec4{ forward, 0.0f }
			});

		m_Target = m_Position + forward;
	}
}