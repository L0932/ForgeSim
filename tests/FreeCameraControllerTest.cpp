#include "FreeCameraController.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace
{
	constexpr float tolerance = 1e-5f;

	[[nodiscard]] bool NearlyEqual(
		float left,
		float right) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}

	[[nodiscard]]
	ForgeSim::Renderer::PerspectiveCamera CreateCamera()
	{
		return {
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};
	}

	[[nodiscard]] glm::vec4 ViewSpaceOrigin(
		const ForgeSim::Renderer::PerspectiveCamera& camera)
	{
		return camera.ViewMatrix() *
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
	}
}

int main()
{
	using ForgeSim::Platform::CursorPosition;
	using ForgeSim::Platform::InputState;
	using ForgeSim::Platform::Key;
	using ForgeSim::Platform::MouseButton;
	using ForgeSim::Sandbox::FreeCameraController;

	// Equal elapsed time should produce equal movement.
	{
		InputState input;
		input.SetKeyState(Key::W, true);

		FreeCameraController controller;

		auto singleUpdateCamera = CreateCamera();
		auto multipleUpdateCamera = CreateCamera();

		controller.Update(
			singleUpdateCamera,
			input,
			0.1f);

		controller.Update(
			multipleUpdateCamera,
			input,
			0.05f);

		controller.Update(
			multipleUpdateCamera,
			input,
			0.05f);

		const glm::vec4 singleUpdateOrigin =
			ViewSpaceOrigin(singleUpdateCamera);

		const glm::vec4 multipleUpdateOrigin =
			ViewSpaceOrigin(multipleUpdateCamera);

		if (!NearlyEqual(
			singleUpdateOrigin.z,
			multipleUpdateOrigin.z))
		{
			std::cerr <<
				"Camera movement depended on update frequency.\n";

			return EXIT_FAILURE;
		}
	}

	// Large frame durations should be clamped.
	{
		InputState input;
		input.SetKeyState(Key::W, true);

		FreeCameraController controller;

		auto boundedCamera = CreateCamera();
		auto longFrameCamera = CreateCamera();

		controller.Update(
			boundedCamera,
			input,
			0.1f);

		controller.Update(
			longFrameCamera,
			input,
			1.0f);

		const glm::vec4 boundedOrigin =
			ViewSpaceOrigin(boundedCamera);

		const glm::vec4 longFrameOrigin =
			ViewSpaceOrigin(longFrameCamera);

		if (!NearlyEqual(
			boundedOrigin.z,
			longFrameOrigin.z))
		{
			std::cerr <<
				"Large frame duration was not clamped.\n";

			return EXIT_FAILURE;
		}
	}

	// Cursor movement should rotate the camera only while
	// camera-look control is active.
	{
		InputState input;

		input.SetCursorPosition(
			CursorPosition{
				.x = 100.0,
				.y = 100.0
			});

		input.SetCursorPosition(
			CursorPosition{
				.x = 150.0,
				.y = 100.0
			});

		FreeCameraController controller;
		auto camera = CreateCamera();

		controller.Update(
			camera,
			input,
			0.016f);

		const glm::vec3 inactiveDirection =
			camera.ForwardDirection();

		if (!NearlyEqual(inactiveDirection.x, 0.0f) ||
			!NearlyEqual(inactiveDirection.y, 0.0f) ||
			!NearlyEqual(inactiveDirection.z, -1.0f))
		{
			std::cerr <<
				"Mouse movement rotated the camera while look was inactive.\n";

			return EXIT_FAILURE;
		}

		input.BeginFrame();

		input.SetMouseButtonState(
			MouseButton::Right,
			true);

		input.SetCursorPosition(
			CursorPosition{
				.x = 200.0,
				.y = 100.0
			});

		controller.Update(
			camera,
			input,
			0.016f);

		const glm::vec3 activeDirection =
			camera.ForwardDirection();

		if (NearlyEqual(activeDirection.x, 0.0f) &&
			NearlyEqual(activeDirection.z, -1.0f))
		{
			std::cerr <<
				"Mouse movement did not rotate the camera while look was active.\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}