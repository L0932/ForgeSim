#include <ForgeSim/Platform/InputState.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
	using ForgeSim::Platform::CursorPosition;
	using ForgeSim::Platform::InputState;
	using ForgeSim::Platform::Key;
	using ForgeSim::Platform::MouseButton;

	InputState input;

	// Key press, hold, and release transitions.
	{
		input.SetKeyState(Key::W, true);

		const auto pressed = input.GetKeyState(Key::W);

		if (!pressed.pressed ||
			!pressed.held ||
			pressed.released)
		{
			std::cerr <<
				"Key press state was incorrect.\n";

			return EXIT_FAILURE;
		}

		input.BeginFrame();

		const auto held = input.GetKeyState(Key::W);

		if (held.pressed ||
			!held.held ||
			held.released)
		{
			std::cerr <<
				"Key held state was incorrect.\n";

			return EXIT_FAILURE;
		}

		// Another down update must not create another press.
		input.SetKeyState(Key::W, true);

		if (input.GetKeyState(Key::W).pressed)
		{
			std::cerr <<
				"Held key produced an additional press.\n";

			return EXIT_FAILURE;
		}

		input.SetKeyState(Key::W, false);

		const auto released = input.GetKeyState(Key::W);

		if (released.pressed ||
			released.held ||
			!released.released)
		{
			std::cerr <<
				"Key release state was incorrect.\n";

			return EXIT_FAILURE;
		}

		input.BeginFrame();

		if (input.GetKeyState(Key::W).released)
		{
			std::cerr <<
				"Key release persisted into another frame.\n";

			return EXIT_FAILURE;
		}
	}

	// Mouse buttons should use the same transition behavior.
	{
		input.SetMouseButtonState(
			MouseButton::Right,
			true);

		const auto pressed =
			input.GetMouseButtonState(
				MouseButton::Right);

		if (!pressed.pressed || !pressed.held)
		{
			std::cerr <<
				"Mouse-button press state was incorrect.\n";

			return EXIT_FAILURE;
		}

		input.SetMouseButtonState(
			MouseButton::Right,
			false);

		const auto released =
			input.GetMouseButtonState(
				MouseButton::Right);

		if (released.held || !released.released)
		{
			std::cerr <<
				"Mouse-button release state was incorrect.\n";

			return EXIT_FAILURE;
		}

		input.BeginFrame();
	}

	// The first cursor position establishes a baseline.
	{
		input.SetCursorPosition(
			CursorPosition{
				.x = 100.0,
				.y = 50.0
			});

		auto delta = input.GetCursorDelta();

		if (delta.x != 0.0 || delta.y != 0.0)
		{
			std::cerr <<
				"Initial cursor position produced movement.\n";

			return EXIT_FAILURE;
		}

		input.SetCursorPosition(
			CursorPosition{
				.x = 110.0,
				.y = 45.0
			});

		input.SetCursorPosition(
			CursorPosition{
				.x = 115.0,
				.y = 40.0
			});

		delta = input.GetCursorDelta();

		if (delta.x != 15.0 || delta.y != -10.0)
		{
			std::cerr <<
				"Cursor movement did not accumulate correctly.\n";

			return EXIT_FAILURE;
		}

		input.BeginFrame();

		delta = input.GetCursorDelta();

		if (delta.x != 0.0 || delta.y != 0.0)
		{
			std::cerr <<
				"Cursor delta persisted into another frame.\n";

			return EXIT_FAILURE;
		}
	}

	// Clearing input should release held state and reset cursor movement.
	{
		input.SetKeyState(Key::A, true);

		input.SetMouseButtonState(
			MouseButton::Right,
			true);

		input.Clear();

		const auto key = input.GetKeyState(Key::A);

		const auto button =
			input.GetMouseButtonState(
				MouseButton::Right);

		if (key.held || !key.released)
		{
			std::cerr <<
				"Clear did not release the held key.\n";

			return EXIT_FAILURE;
		}

		if (button.held || !button.released)
		{
			std::cerr <<
				"Clear did not release the held mouse button.\n";

			return EXIT_FAILURE;
		}
	}

	// Resetting cursor tracking should prevent a discontinuity.
	{
		input.ResetCursorTracking();

		input.SetCursorPosition(
			CursorPosition{
				.x = 500.0,
				.y = 300.0
			});

		auto delta = input.GetCursorDelta();

		if (delta.x != 0.0 || delta.y != 0.0)
		{
			std::cerr <<
				"Cursor reset produced unexpected movement.\n";

			return EXIT_FAILURE;
		}

		input.SetCursorPosition(
			CursorPosition{
				.x = 504.0,
				.y = 297.0
			});

		delta = input.GetCursorDelta();

		if (delta.x != 4.0 || delta.y != -3.0)
		{
			std::cerr <<
				"Cursor movement after reset was incorrect.\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}