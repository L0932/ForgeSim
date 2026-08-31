#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Core
{
	Timer::Timer() noexcept
		: m_startTime(Clock::now())
	{}

	Timer::Duration Timer::Elapsed() const noexcept
	{
		return std::chrono::duration_cast<Duration>(
			Clock::now() - m_startTime
		);
	}
	void Timer::Reset() noexcept
	{
		m_startTime = Clock::now();
	}
}