#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Core
{
	// Initializes the timer to the current time.
	Timer::Timer() noexcept
		: m_startTime(Clock::now())
	{}

	// Returns the elapsed time since the last restart.
	Timer::Duration Timer::Elapsed() const noexcept
	{
		return std::chrono::duration_cast<Duration>(
			Clock::now() - m_startTime
		);
	}
	
	// Resets the timer to the current time.
	void Timer::Reset() noexcept
	{
		m_startTime = Clock::now();
	}

	// Returns the elapsed time since the last restart and resets the timer.
	Timer::Duration Timer::Restart() noexcept
	{
		const auto now = Clock::now();
		const auto elapsed = std::chrono::duration_cast<Duration>(
			now - m_startTime
		);

		m_startTime = now;

		return elapsed;
	}
}