#pragma once

#include <chrono>

namespace ForgeSim::Core
{
	class Timer final
	{
	public:
		using Duration = std::chrono::nanoseconds;

		Timer() noexcept;

		[[nodiscard]] Duration Elapsed() const noexcept;

		void Reset() noexcept;

	private:
		using Clock = std::chrono::steady_clock;

		Clock::time_point m_startTime;
	};
}