#include <ForgeSim/Platform/InputState.hpp>

#include <cassert>

namespace ForgeSim::Platform
{
	namespace
	{
		void UpdateButtonState(
			ButtonState& state,
			bool isDown) noexcept
		{
			if (isDown)
			{
				if (!state.held)
				{
					state.pressed = true;
				}

				state.held = true;
				return;
			}

			if (state.held)
			{
				state.released = true;
			}

			state.held = false;
		}

		void BeginButtonFrame(
			ButtonState& state) noexcept
		{
			state.pressed = false;
			state.released = false;
		}

		void ClearButtonState(
			ButtonState& state) noexcept
		{
			state.released =
				state.released || state.held;

			state.pressed = false;
			state.held = false;
		}
	}

	void InputState::BeginFrame() noexcept
	{
		for (ButtonState& state : m_KeyStates)
		{
			BeginButtonFrame(state);
		}

		for (ButtonState& state : m_MouseButtonStates)
		{
			BeginButtonFrame(state);
		}

		m_CursorDelta = {};
	}

	void InputState::ResetCursorTracking() noexcept
	{
		m_CursorDelta = {};
		m_HasCursorPosition = false;
	}

	void InputState::SetKeyState(
		Key key,
		bool isDown) noexcept
	{
		const std::size_t index =
			static_cast<std::size_t>(key);

		assert(index < m_KeyStates.size());

		UpdateButtonState(
			m_KeyStates[index],
			isDown);
	}

	void InputState::SetMouseButtonState(
		MouseButton button,
		bool isDown) noexcept
	{
		const std::size_t index =
			static_cast<std::size_t>(button);

		assert(index < m_MouseButtonStates.size());

		UpdateButtonState(
			m_MouseButtonStates[index],
			isDown);
	}

	void InputState::SetCursorPosition(
		CursorPosition position) noexcept
	{
		if (m_HasCursorPosition)
		{
			m_CursorDelta.x +=
				position.x - m_CursorPosition.x;

			m_CursorDelta.y +=
				position.y - m_CursorPosition.y;
		}
		else
		{
			m_HasCursorPosition = true;
		}

		m_CursorPosition = position;
	}

	void InputState::Clear() noexcept
	{
		for (ButtonState& state : m_KeyStates)
		{
			ClearButtonState(state);
		}

		for (ButtonState& state : m_MouseButtonStates)
		{
			ClearButtonState(state);
		}

		ResetCursorTracking();
	}

	ButtonState InputState::GetKeyState(
		Key key) const noexcept
	{
		const std::size_t index =
			static_cast<std::size_t>(key);

		assert(index < m_KeyStates.size());

		return m_KeyStates[index];
	}

	ButtonState InputState::GetMouseButtonState(
		MouseButton button) const noexcept
	{
		const std::size_t index =
			static_cast<std::size_t>(button);

		assert(index < m_MouseButtonStates.size());

		return m_MouseButtonStates[index];
	}

	CursorPosition InputState::GetCursorPosition()
		const noexcept
	{
		return m_CursorPosition;
	}

	CursorDelta InputState::GetCursorDelta()
		const noexcept
	{
		return m_CursorDelta;
	}
}