#pragma once

#include <chrono>
#include <cstdint>

namespace ForgeSim::Core
{
	class FixedStepScheduler final
	{
	public:
		using Duration = std::chrono::nanoseconds;

		struct AddTimeResult
		{
			std::uint32_t stepCount;
			Duration droppedTime;
		};

		explicit FixedStepScheduler(
			Duration fixedStep,
			std::uint32_t maximumStepsPerFrame) noexcept;

		[[nodiscard]] AddTimeResult AddTime(Duration deltaTime) noexcept;
		[[nodiscard]] Duration FixedStep() const noexcept;
		[[nodiscard]] double InterpolationAlpha() const noexcept;

		void Reset() noexcept;

	private:
		Duration m_fixedStep;
		Duration m_accumulatedTime{};
		std::uint32_t m_maximumStepsPerFrame;
	};
}