#pragma once

#include "SceneLight.hpp"

#include <cmath>
#include <optional>
#include <span>

#include <ForgeSim/Renderer/Picking.hpp>

namespace ForgeSim::Sandbox
{
	struct LightPickTarget
	{
		SceneLightId id = InvalidSceneLightId;
		Renderer::BoundingSphere bounds;
	};

	struct LightPickHit
	{
		SceneLightId id;
		float distance;
	};

	[[nodiscard]] inline std::optional<LightPickHit>
		PickClosestLight(
			const Renderer::Ray& ray,
			std::span<const LightPickTarget> targets) noexcept
	{
		std::optional<LightPickHit> closestHit;

		for (const LightPickTarget& target : targets)
		{
			if (target.id == InvalidSceneLightId)
			{
				continue;
			}

			const auto distance =
				Renderer::IntersectRaySphere(
					ray,
					target.bounds);

			if (!distance ||
				!std::isfinite(*distance) ||
				*distance < 0.0f)
			{
				continue;
			}

			const bool isCloser =
				!closestHit ||
				*distance < closestHit->distance;

			const bool winsTie =
				closestHit &&
				*distance == closestHit->distance &&
				target.id < closestHit->id;

			if (isCloser || winsTie)
			{
				closestHit = LightPickHit{
					.id = target.id,
					.distance = *distance
				};
			}
		}

		return closestHit;
	}
}