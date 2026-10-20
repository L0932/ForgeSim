#pragma once

#include "SceneLightValidation.hpp"

namespace ForgeSim::Sandbox
{
	// Empty optional means the candidate was accepted.
	// Rejected edits leave the target unchanged.
	// Editing cannot change a scene-owned light ID.
	[[nodiscard]] std::optional<LightValidationError> TryApplyLightEdit(
		DirectionalLight& target,
		const DirectionalLight& candidate) noexcept;

	[[nodiscard]] std::optional<LightValidationError> TryApplyLightEdit(
		PointLight& target,
		const PointLight& candidate) noexcept;

	[[nodiscard]] std::optional<LightValidationError> TryApplyLightEdit(
		SpotLight& target,
		const SpotLight& candidate) noexcept;
}