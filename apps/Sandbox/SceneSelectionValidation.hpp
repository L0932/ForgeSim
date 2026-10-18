#pragma once

#include "SandboxScene.hpp"
#include "SceneSelection.hpp"

#include <algorithm>

namespace ForgeSim::Sandbox
{
	inline void ClearMissingSceneSelection(
		SceneSelection& selection,
		const SandboxScene& scene) noexcept
	{
		if (const auto objectId =
			selection.SelectedObjectId())
		{
			if (scene.FindObject(*objectId) == nullptr)
			{
				selection.Clear();
			}

			return;
		}

		const auto lightId =
			selection.SelectedLightId();

		if (!lightId)
		{
			return;
		}

		if (scene.MainDirectionalLight().id == *lightId)
		{
			return;
		}

		const auto pointLights = scene.PointLights();

		const bool pointLightExists =
			std::any_of(
				pointLights.begin(),
				pointLights.end(),
				[lightId](const PointLight& light)
				{
					return light.id == *lightId;
				});

		if (pointLightExists)
		{
			return;
		}

		const auto spotLights = scene.SpotLights();

		const bool spotLightExists =
			std::any_of(
				spotLights.begin(),
				spotLights.end(),
				[lightId](const SpotLight& light)
				{
					return light.id == *lightId;
				});

		if (!spotLightExists)
		{
			selection.Clear();
		}
	}
}