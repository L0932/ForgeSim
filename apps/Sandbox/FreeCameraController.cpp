#include "FreeCameraController.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	FreeCameraController::FreeCameraController(
		const FreeCameraControllerSpecification& specification)
		noexcept
		: m_Specification(specification)
	{}

	[[nodiscard]]
	std::optional<Platform::CursorMode> FreeCameraController::RequestedCursorMode(
		const Platform::InputState& input) const noexcept
	{
		const auto lookButtonState =
			input.GetMouseButtonState(
				Platform::MouseButton::Right);

		if (lookButtonState.pressed)
		{
			return Platform::CursorMode::Captured;
		}

		if (lookButtonState.released)
		{
			return Platform::CursorMode::Normal;
		}

		return std::nullopt;
	}

    void FreeCameraController::SetMovementSpeed(float speed)
    {
        if (!std::isfinite(speed) || speed < 0.0001f || speed > 10000.0f)
            throw std::invalid_argument("Camera speed must be between 0.0001 and 10000 units per second.");
        m_Specification.movementSpeed = speed;
    }

    bool FreeCameraController::AdjustMovementSpeed(double wheelDelta) noexcept
    {
        if (!std::isfinite(wheelDelta) || wheelDelta == 0.0) return false;
        const double factor = std::pow(1.25, std::clamp(wheelDelta, -128.0, 128.0));
        const float speed = static_cast<float>(std::clamp(
            static_cast<double>(m_Specification.movementSpeed) * factor, 0.0001, 10000.0));
        if (speed == m_Specification.movementSpeed) return false;
        m_Specification.movementSpeed = speed;
        return true;
    }

	void FreeCameraController::Update(
		ForgeSim::Renderer::PerspectiveCamera& camera,
		const ForgeSim::Platform::InputState& input,
		float deltaSeconds) const
	{

		constexpr float maximumCameraDeltaSeconds = 0.1f;

		const float clampedDeltaSeconds =
			std::min(deltaSeconds, maximumCameraDeltaSeconds);

		const float boundedDeltaSeconds = std::clamp(
			clampedDeltaSeconds,
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
            const float slowFactor = input.GetKeyState(Platform::Key::LeftControl).held ? 0.1f : 1.0f;
			camera.Translate(
				glm::normalize(movementDirection) *
				m_Specification.movementSpeed * slowFactor *
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