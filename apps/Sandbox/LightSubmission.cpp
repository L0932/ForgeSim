#include "LightSubmission.hpp"

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

		[[nodiscard]] bool HasValidBaseProperties(
			const glm::vec3& position,
			const glm::vec3& color,
			float intensity,
			float range) noexcept
		{
			return
				IsFinite(position) &&
				IsFinite(color) &&
				color.x >= 0.0f &&
				color.y >= 0.0f &&
				color.z >= 0.0f &&
				std::isfinite(intensity) &&
				intensity > 0.0f &&
				std::isfinite(range) &&
				range > 0.0f;
		}
	}

	PointLightSubmission PreparePointLights(
		std::span<const PointLight> lights)
	{
		PointLightSubmission submission;

		for (const PointLight& light : lights)
		{
			if (submission.count == submission.lights.size())
			{
				break;
			}

			if (!light.enabled ||
				!HasValidBaseProperties(
					light.position,
					light.color,
					light.intensity,
					light.range))
			{
				continue;
			}

			const glm::vec3 radiance =
				light.color * light.intensity;

			if (!IsFinite(radiance))
			{
				continue;
			}

			submission.lights[submission.count] =
				PreparedPointLight{
					.position = light.position,
					.radiance = radiance,
					.range = light.range
			};

			++submission.count;
		}

		return submission;
	}

	SpotLightSubmission PrepareSpotLights(
		std::span<const SpotLight> lights)
	{
		SpotLightSubmission submission;

		for (const SpotLight& light : lights)
		{
			if (submission.count == submission.lights.size())
			{
				break;
			}

			if (!light.enabled ||
				!HasValidBaseProperties(
					light.position,
					light.color,
					light.intensity,
					light.range) ||
				!IsFinite(light.emissionDirection) ||
				!std::isfinite(light.innerConeAngleRadians) ||
				!std::isfinite(light.outerConeAngleRadians) ||
				light.innerConeAngleRadians < 0.0f ||
				light.outerConeAngleRadians <=
				light.innerConeAngleRadians ||
				light.outerConeAngleRadians >=
				std::numbers::pi_v<float> *0.5f)
			{
				continue;
			}

			const float directionLengthSquared =
				glm::dot(
					light.emissionDirection,
					light.emissionDirection);

			if (!std::isfinite(directionLengthSquared) ||
				directionLengthSquared < 1e-12f)
			{
				continue;
			}

			const glm::vec3 radiance =
				light.color * light.intensity;

			if (!IsFinite(radiance))
			{
				continue;
			}

			const float innerConeCos =
				std::cos(light.innerConeAngleRadians);

			const float outerConeCos =
				std::cos(light.outerConeAngleRadians);

			if (innerConeCos <= outerConeCos)
			{
				continue;
			}

			submission.lights[submission.count] =
				PreparedSpotLight{
					.position = light.position,
					.emissionDirection =
						light.emissionDirection /
						std::sqrt(directionLengthSquared),
					.radiance = radiance,
					.range = light.range,
					.innerConeCos = innerConeCos,
					.outerConeCos = outerConeCos
			};

			++submission.count;
		}

		return submission;
	}
}