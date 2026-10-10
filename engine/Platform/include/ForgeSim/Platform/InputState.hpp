#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ForgeSim::Platform
{
	enum class CursorMode : std::uint8_t
	{
		Normal,
		Captured
	};

	enum class Key : std::uint8_t
	{
		W,
		A,
		S,
		D,
		Space,
		LeftControl,
		Count
	};

	enum class MouseButton : std::uint8_t
	{
		Left,
		Right,
		Count
	};

	struct ButtonState
	{
		bool pressed = false;
		bool held = false;
		bool released = false;
	};

	struct CursorPosition
	{
		double x = 0.0;
		double y = 0.0;
	};

	struct CursorDelta
	{
		double x = 0.0;
		double y = 0.0;
	};

	class InputState final
	{
	public:
		void BeginFrame() noexcept;

		void ResetCursorTracking() noexcept;

		void SetKeyState(
			Key key,
			bool isDown) noexcept;

		void SetMouseButtonState(
			MouseButton button,
			bool isDown) noexcept;

		void SetCursorPosition(
			CursorPosition position) noexcept;

		void Clear() noexcept;

		[[nodiscard]] ButtonState GetKeyState(
			Key key) const noexcept;

		[[nodiscard]] ButtonState GetMouseButtonState(
			MouseButton button) const noexcept;

		[[nodiscard]] CursorPosition GetCursorPosition()
			const noexcept;

		[[nodiscard]] CursorDelta GetCursorDelta()
			const noexcept;

	private:
		static constexpr std::size_t keyCount =
			static_cast<std::size_t>(Key::Count);

		static constexpr std::size_t mouseButtonCount =
			static_cast<std::size_t>(MouseButton::Count);

		std::array<ButtonState, keyCount> m_KeyStates{};
		std::array<ButtonState, mouseButtonCount>
			m_MouseButtonStates{};

		CursorPosition m_CursorPosition{};
		CursorDelta m_CursorDelta{};

		bool m_HasCursorPosition = false;
	};
}