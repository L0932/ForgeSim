#include <ForgeSim/Core/RuntimeDiagnostics.hpp>

namespace ForgeSim::Core
{
	std::optional<RuntimeDiagnosticsSnapshot>
		RuntimeDiagnostics::AddFrame(
			Timer::Duration frameDelta,
			const FixedStepScheduler::AddTimeResult&
			fixedStepResult) noexcept
	{
		if (!m_Enabled)
		{
			return std::nullopt;
		}

		m_FixedUpdateCount +=
			fixedStepResult.stepCount;

		if (fixedStepResult.droppedTime >
			FixedStepScheduler::Duration::zero())
		{
			++m_CatchUpLimitedFrameCount;

			m_DroppedSimulationTime +=
				fixedStepResult.droppedTime;
		}

		const auto frameStatisticsSnapshot =
			m_FrameStatistics.AddFrame(frameDelta);

		if (!frameStatisticsSnapshot)
		{
			return std::nullopt;
		}

		const RuntimeDiagnosticsSnapshot snapshot{
			.frameStatistics =
				*frameStatisticsSnapshot,

			.fixedUpdateCount =
				m_FixedUpdateCount,

			.catchUpLimitedFrameCount =
				m_CatchUpLimitedFrameCount,

			.droppedSimulationTime =
				m_DroppedSimulationTime
		};

		Reset();

		return snapshot;
	}

	void RuntimeDiagnostics::SetEnabled(
		bool enabled) noexcept
	{
		if (m_Enabled == enabled)
		{
			return;
		}

		m_Enabled = enabled;
		Reset();
	}

	bool RuntimeDiagnostics::IsEnabled() const noexcept
	{
		return m_Enabled;
	}

	void RuntimeDiagnostics::Reset() noexcept
	{
		m_FrameStatistics.Reset();

		m_FixedUpdateCount = 0;
		m_CatchUpLimitedFrameCount = 0;
		m_DroppedSimulationTime = {};
	}
}