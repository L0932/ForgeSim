#include <ForgeSim/Core/FrameStatistics.hpp>

#include <chrono>

namespace ForgeSim::Core
{
	std::optional<FrameStatisticsSnapshot> FrameStatistics::AddFrame(
		Timer::Duration frameDelta) noexcept
	{
        m_accumulatedTime += frameDelta;
        ++m_frameCount;

        if (m_accumulatedTime < std::chrono::seconds{ 1 })
        {
            return std::nullopt;
        }

        const double elapsedSeconds =
            std::chrono::duration<double>{ m_accumulatedTime }.count();

        const double elapsedMilliseconds =
            std::chrono::duration<double, std::milli>{
                m_accumulatedTime
        }.count();

        const FrameStatisticsSnapshot snapshot{
            .framesPerSecond =
                static_cast<double>(m_frameCount) / elapsedSeconds,

            .averageFrameTimeMilliseconds =
                elapsedMilliseconds / static_cast<double>(m_frameCount),

            .frameCount = m_frameCount
        };

        Reset();

        return snapshot;
	}

	void FrameStatistics::Reset() noexcept
	{
		m_accumulatedTime = {};
		m_frameCount = 0;
	}
}