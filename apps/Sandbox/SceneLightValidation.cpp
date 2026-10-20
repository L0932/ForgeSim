#include "SceneLightValidation.hpp"

#include <cmath>
#include <numbers>

#include <glm/geometric.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] bool IsFinite(
			const glm::vec3& value) noexcept
		{
			return std::isfinite(value.x) &&
				std::isfinite(value.y) &&
				std::isfinite(value.z);
		}

		[[nodiscard]] bool IsUsableDirection(
			const glm::vec3& direction) noexcept
		{
			if (!IsFinite(direction))
			{
				return false;
			}

			const float lengthSquared =
				glm::dot(direction, direction);

			// Match the current consumers' normalization limits.
			return std::isfinite(lengthSquared) &&
				lengthSquared >= 1e-12f;
		}

		template<typename Light>
		[[nodiscard]] std::optional<LightValidationError>
			ValidateCommon(
				const Light& light,
				SceneLightKind kind) noexcept
		{
			if (!IsFinite(light.position))
			{
				return LightValidationError{
					kind,
					light.id,
					"position",
					"Position components must be finite."
				};
			}

			if (!IsFinite(light.color) ||
				light.color.x < 0.0f ||
				light.color.y < 0.0f ||
				light.color.z < 0.0f)
			{
				return LightValidationError{
					kind,
					light.id,
					"color",
					"Color components must be finite and nonnegative."
				};
			}

			if (!std::isfinite(light.intensity) ||
				light.intensity < 0.0f)
			{
				return LightValidationError{
					kind,
					light.id,
					"intensity",
					"Intensity must be finite and nonnegative."
				};
			}

			if (!IsFinite(light.color * light.intensity))
			{
				return LightValidationError{
					kind,
					light.id,
					"intensity",
					"Color multiplied by intensity exceeds "
					"the supported numeric range."
				};
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<LightValidationError>
			ValidateRange(
				float range,
				SceneLightKind kind,
				SceneLightId id) noexcept
		{
			if (!std::isfinite(range) || range <= 0.0f)
			{
				return LightValidationError{
					kind,
					id,
					"range",
					"Range must be finite and positive."
				};
			}

			return std::nullopt;
		}
	}

	std::optional<LightValidationError> ValidateLight(
		const DirectionalLight& light) noexcept
	{
		constexpr SceneLightKind kind =
			SceneLightKind::Directional;

		if (const auto error = ValidateCommon(light, kind))
		{
			return error;
		}

		if (!IsUsableDirection(light.directionToLight))
		{
			return LightValidationError{
				kind,
				light.id,
				"directionToLight",
				"Direction must be finite and safely normalizable "
				"with length at least 0.000001."
			};
		}

		return std::nullopt;
	}

	std::optional<LightValidationError> ValidateLight(
		const PointLight& light) noexcept
	{
		constexpr SceneLightKind kind =
			SceneLightKind::Point;

		if (const auto error = ValidateCommon(light, kind))
		{
			return error;
		}

		return ValidateRange(light.range, kind, light.id);
	}

	std::optional<LightValidationError> ValidateLight(
		const SpotLight& light) noexcept
	{
		constexpr SceneLightKind kind =
			SceneLightKind::Spot;

		if (const auto error = ValidateCommon(light, kind))
		{
			return error;
		}

		if (const auto error =
			ValidateRange(light.range, kind, light.id))
		{
			return error;
		}

		if (!IsUsableDirection(light.emissionDirection))
		{
			return LightValidationError{
				kind,
				light.id,
				"emissionDirection",
				"Direction must be finite and safely normalizable "
				"with length at least 0.000001."
			};
		}

		if (!std::isfinite(light.innerConeAngleRadians) ||
			light.innerConeAngleRadians < 0.0f)
		{
			return LightValidationError{
				kind,
				light.id,
				"innerConeAngleRadians",
				"Inner cone half-angle must be finite and nonnegative."
			};
		}

		if (!std::isfinite(light.outerConeAngleRadians) ||
			light.outerConeAngleRadians <=
			light.innerConeAngleRadians ||
			light.outerConeAngleRadians >=
			std::numbers::pi_v<float> *0.5f)
		{
			return LightValidationError{
				kind,
				light.id,
				"outerConeAngleRadians",
				"Outer cone half-angle must be greater than "
				"the inner angle and less than pi / 2."
			};
		}

		const float innerCos =
			std::cos(light.innerConeAngleRadians);

		const float outerCos =
			std::cos(light.outerConeAngleRadians);

		if (innerCos <= outerCos)
		{
			return LightValidationError{
				kind,
				light.id,
				"outerConeAngleRadians",
				"Cone angles are too close to produce distinct "
				"float cosine values."
			};
		}

		return std::nullopt;
	}
}