#include "FreeCameraController.hpp"

#include <algorithm>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	FreeCameraController::FreeCameraController(
		const FreeCameraControllerSpecification& specification)
		noexcept
		: m_Specification(specification)
	{}

	void FreeCameraController::Update(
		ForgeSim::Renderer::PerspectiveCamera& camera,
		const ForgeSim::Platform::InputState& input,
		float deltaSeconds) const
	{
		const float boundedDeltaSeconds = std::clamp(
			deltaSeconds,
			0.0f,
			m_Specification.maximumDeltaSeconds);

		glm::vec3 movementDirection{ 0.0f };

		if (input.GetKeyState(
			ForgeSim::Platform::Key::W).held)
		{
			movementDirection +=
				camera.ForwardDirection();
		}

		if (input.GetKeyState(
			ForgeSim::Platform::Key::S).held)
		{
			movementDirection -=
				camera.ForwardDirection();
		}

		if (input.GetKeyState(
			ForgeSim::Platform::Key::D).held)
		{
			movementDirection +=
				camera.RightDirection();
		}

		if (input.GetKeyState(
			ForgeSim::Platform::Key::A).held)
		{
			movementDirection -=
				camera.RightDirection();
		}

		if (glm::dot(
			movementDirection,
			movementDirection) > 0.0f)
		{
			camera.Translate(
				glm::normalize(movementDirection) *
				m_Specification.movementSpeed *
				boundedDeltaSeconds);
		}

		const auto cameraLookState =
			input.GetMouseButtonState(
				ForgeSim::Platform::MouseButton::Right);

		if (cameraLookState.held)
		{
			const auto cursorDelta =
				input.GetCursorDelta();

			camera.Rotate(
				-static_cast<float>(cursorDelta.x) *
				m_Specification.mouseSensitivity,
				-static_cast<float>(cursorDelta.y) *
				m_Specification.mouseSensitivity);
		}
	}
}