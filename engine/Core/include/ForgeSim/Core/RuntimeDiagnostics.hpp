#pragma once

#include <cstdint>
#include <optional>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>
#include <ForgeSim/Core/Timer.hpp>

namespace ForgeSim::Core
{
	struct RuntimeDiagnosticsSnapshot
	{
		FrameStatisticsSnapshot frameStatistics;

		std::uint64_t fixedUpdateCount = 0;
		std::uint64_t catchUpLimitedFrameCount = 0;

		FixedStepScheduler::Duration droppedSimulationTime{};
	};

	class RuntimeDiagnostics final
	{
	public:
		[[nodiscard]] std::optional<RuntimeDiagnosticsSnapshot>
			AddFrame(
				Timer::Duration frameDelta,
				const FixedStepScheduler::AddTimeResult&
				fixedStepResult) noexcept;

		void SetEnabled(bool enabled) noexcept;

		[[nodiscard]] bool IsEnabled() const noexcept;

		void Reset() noexcept;

	private:
		FrameStatistics m_FrameStatistics;

		std::uint64_t m_FixedUpdateCount = 0;
		std::uint64_t m_CatchUpLimitedFrameCount = 0;

		FixedStepScheduler::Duration
			m_DroppedSimulationTime{};

		bool m_Enabled = true;
	};
}