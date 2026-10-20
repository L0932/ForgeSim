#include "LightSubmission.hpp"
#include "SceneLightValidation.hpp"

#include <cmath>

#include <glm/geometric.hpp>

namespace ForgeSim::Sandbox
{
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
				light.intensity == 0.0f ||
				ValidateLight(light).has_value())
			{
				continue;
			}

			submission.lights[submission.count] =
				PreparedPointLight{
					.position = light.position,
					.radiance = light.color * light.intensity,
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
				light.intensity == 0.0f ||
				ValidateLight(light).has_value())
			{
				continue;
			}

			const float directionLengthSquared =
				glm::dot(
					light.emissionDirection,
					light.emissionDirection);

			submission.lights[submission.count] =
				PreparedSpotLight{
					.position = light.position,
					.emissionDirection =
						light.emissionDirection /
						std::sqrt(directionLengthSquared),
					.radiance = light.color * light.intensity,
					.range = light.range,
					.innerConeCos =
						std::cos(light.innerConeAngleRadians),
					.outerConeCos =
						std::cos(light.outerConeAngleRadians)
			};

			++submission.count;
		}

		return submission;
	}
}