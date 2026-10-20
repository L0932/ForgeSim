#pragma once

#include "SceneLight.hpp"

#include <array>
#include <cstddef>
#include <span>

namespace ForgeSim::Sandbox
{
	inline constexpr std::size_t maximumSubmittedPointLights = 16;
	inline constexpr std::size_t maximumSubmittedSpotLights = 16;

	struct PreparedPointLight
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 radiance{ 0.0f };
		float range = 0.0f;
	};

	struct PreparedSpotLight
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 emissionDirection{ 0.0f };
		glm::vec3 radiance{ 0.0f };
		float range = 0.0f;
		float innerConeCos = 0.0f;
		float outerConeCos = 0.0f;
	};

	struct PointLightSubmission
	{
		std::array<
			PreparedPointLight,
			maximumSubmittedPointLights> lights{};

		std::size_t count = 0;
	};

	struct SpotLightSubmission
	{
		std::array<
			PreparedSpotLight,
			maximumSubmittedSpotLights> lights{};

		std::size_t count = 0;
	};

	// Select the first enabled, valid lights in scene order.
	// Invalid lights do not consume submission capacity.
	// Scene data is never modified.
	[[nodiscard]] PointLightSubmission PreparePointLights(
		std::span<const PointLight> lights);

	[[nodiscard]] SpotLightSubmission PrepareSpotLights(
		std::span<const SpotLight> lights);
}