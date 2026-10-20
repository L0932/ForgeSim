#include "SceneLightEditing.hpp"

namespace ForgeSim::Sandbox
{
	namespace
	{
		template<typename Light>
		[[nodiscard]] std::optional<LightValidationError>
			TryApplyLightEditImpl(
				Light& target,
				const Light& candidate,
				SceneLightKind kind) noexcept
		{
			if (candidate.id != target.id)
			{
				return LightValidationError{
					kind,
					target.id,
					"id",
					"Editing cannot change a scene-owned light ID."
				};
			}

			if (const auto error = ValidateLight(candidate))
			{
				return error;
			}

			target = candidate;
			return std::nullopt;
		}
	}

	std::optional<LightValidationError> TryApplyLightEdit(
		DirectionalLight& target,
		const DirectionalLight& candidate) noexcept
	{
		return TryApplyLightEditImpl(
			target,
			candidate,
			SceneLightKind::Directional);
	}

	std::optional<LightValidationError> TryApplyLightEdit(
		PointLight& target,
		const PointLight& candidate) noexcept
	{
		return TryApplyLightEditImpl(
			target,
			candidate,
			SceneLightKind::Point);
	}

	std::optional<LightValidationError> TryApplyLightEdit(
		SpotLight& target,
		const SpotLight& candidate) noexcept
	{
		return TryApplyLightEditImpl(
			target,
			candidate,
			SceneLightKind::Spot);
	}
}