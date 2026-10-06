#include <ForgeSim/Core/FixedStepScheduler.hpp>

#include <algorithm>
#include <cassert>

namespace ForgeSim::Core
{
	FixedStepScheduler::FixedStepScheduler(
		Duration fixedStep,
		std::uint32_t maximumStepsPerFrame) noexcept
		: m_fixedStep(fixedStep)
		, m_maximumStepsPerFrame(maximumStepsPerFrame)
	{
		assert(fixedStep > Duration::zero());
		assert(maximumStepsPerFrame > 0);
	}

	FixedStepScheduler::Duration FixedStepScheduler::FixedStep() const noexcept
	{
		return m_fixedStep;
	}

	FixedStepScheduler::AddTimeResult FixedStepScheduler::AddTime(Duration deltaTime) noexcept
	{
		assert(deltaTime >= Duration::zero());

		m_accumulatedTime += deltaTime;

		const auto availableSteps = 
			m_accumulatedTime / m_fixedStep;

		const auto executedSteps = std::min(
			availableSteps, 
			static_cast<Duration::rep>(m_maximumStepsPerFrame));

		const auto droppedSteps = 
			availableSteps - executedSteps;

		const Duration droppedTime = 
			m_fixedStep * droppedSteps;

		m_accumulatedTime -= 
			m_fixedStep * availableSteps;

		return {
			.stepCount = static_cast<std::uint32_t>(executedSteps),
			.droppedTime = droppedTime
		};
	}

	double FixedStepScheduler::InterpolationAlpha() const noexcept
	{
		return static_cast<double>(m_accumulatedTime.count()) / 
			static_cast<double>(m_fixedStep.count());
	}

	void FixedStepScheduler::Reset() noexcept
	{
		m_accumulatedTime = Duration::zero();
	}
}