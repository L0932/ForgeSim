#pragma once

#include <optional>

#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Core
{
	struct FrameStatisticsSnapshot
	{
		double framesPerSecond;
		double averageFrameTimeMilliseconds;
		std::uint64_t frameCount;
	};

	class FrameStatistics final
	{
	public:
		[[nodiscard]]
		std::optional<FrameStatisticsSnapshot> AddFrame(
			Timer::Duration frameDelta) noexcept;

		void Reset() noexcept;

	private:
		Timer::Duration m_accumulatedTime{};
		std::uint64_t m_frameCount = 0;
	};
}