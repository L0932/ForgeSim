#pragma once

#include "SceneLight.hpp"

#include <optional>
#include <string_view>

namespace ForgeSim::Sandbox
{
	enum class SceneLightKind
	{
		Directional,
		Point,
		Spot
	};

	struct LightValidationError
	{
		SceneLightKind kind;
		SceneLightId lightId;
		std::string_view property;
		std::string_view message;
	};

	// Empty optional means valid.
	// Validation never modifies or normalizes scene data.
	[[nodiscard]] std::optional<LightValidationError> ValidateLight(
		const DirectionalLight& light) noexcept;

	[[nodiscard]] std::optional<LightValidationError> ValidateLight(
		const PointLight& light) noexcept;

	[[nodiscard]] std::optional<LightValidationError> ValidateLight(
		const SpotLight& light) noexcept;
}