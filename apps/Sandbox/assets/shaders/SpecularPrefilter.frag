#version 460 core

const float pi = 3.14159265359;
const uint sampleCount = 512u;

in vec3 vLocalPosition;

uniform samplerCube uEnvironmentMap;
uniform float uEnvironmentResolution;
uniform float uRoughness;

out vec4 fragColor;

float RadicalInverseVanDerCorput(uint bits)
{
	bits =
		(bits << 16u) |
		(bits >> 16u);

	bits =
		((bits & 0x55555555u) << 1u) |
		((bits & 0xAAAAAAAAu) >> 1u);

	bits =
		((bits & 0x33333333u) << 2u) |
		((bits & 0xCCCCCCCCu) >> 2u);

	bits =
		((bits & 0x0F0F0F0Fu) << 4u) |
		((bits & 0xF0F0F0F0u) >> 4u);

	bits =
		((bits & 0x00FF00FFu) << 8u) |
		((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(
	uint sampleIndex,
	uint numberOfSamples)
{
	return vec2(
		float(sampleIndex) /
			float(numberOfSamples),
		RadicalInverseVanDerCorput(sampleIndex));
}

float DistributionGGX(
	vec3 normal,
	vec3 halfwayDirection,
	float roughness)
{
	float alpha =
		max(
			roughness * roughness,
			0.001);

	float alphaSquared =
		alpha * alpha;

	float normalDotHalfway =
		max(
			dot(normal, halfwayDirection),
			0.0);

	float normalDotHalfwaySquared =
		normalDotHalfway *
		normalDotHalfway;

	float denominator =
		normalDotHalfwaySquared *
		(alphaSquared - 1.0) +
		1.0;

	return alphaSquared /
		max(
			pi * denominator * denominator,
			0.0001);
}

vec3 ImportanceSampleGGX(
	vec2 samplePoint,
	vec3 normal,
	float roughness)
{
	float alpha =
		max(
			roughness * roughness,
			0.001);

	float phi =
		2.0 *
		pi *
		samplePoint.x;

	float cosineTheta =
		sqrt(
			(1.0 - samplePoint.y) /
			(
				1.0 +
				(alpha * alpha - 1.0) *
				samplePoint.y
			));

	float sineTheta =
		sqrt(
			max(
				1.0 -
				cosineTheta *
				cosineTheta,
				0.0));

	vec3 halfwayTangent = vec3(
		cos(phi) * sineTheta,
		sin(phi) * sineTheta,
		cosineTheta);

	vec3 helperDirection =
		abs(normal.z) < 0.999
		? vec3(0.0, 0.0, 1.0)
		: vec3(1.0, 0.0, 0.0);

	vec3 tangent =
		normalize(
			cross(
				helperDirection,
				normal));

	vec3 bitangent =
		cross(
			normal,
			tangent);

	return normalize(
		tangent * halfwayTangent.x +
		bitangent * halfwayTangent.y +
		normal * halfwayTangent.z);
}

void main()
{
	vec3 normal =
		normalize(vLocalPosition);

	// During prefiltering, the view and reflection directions
	// are assumed to match the cubemap direction.
	vec3 reflectionDirection = normal;
	vec3 viewDirection = reflectionDirection;

	if (uRoughness <= 0.001)
	{
		fragColor = vec4(
			textureLod(
				uEnvironmentMap,
				reflectionDirection,
				0.0).rgb,
			1.0);

		return;
	}

	vec3 prefilteredColor =
		vec3(0.0);

	float accumulatedWeight = 0.0;

	for (uint sampleIndex = 0u;
		sampleIndex < sampleCount;
		++sampleIndex)
	{
		vec2 samplePoint =
			Hammersley(
				sampleIndex,
				sampleCount);

		vec3 halfwayDirection =
			ImportanceSampleGGX(
				samplePoint,
				normal,
				uRoughness);

		vec3 lightDirection =
			normalize(
				2.0 *
				dot(
					viewDirection,
					halfwayDirection) *
				halfwayDirection -
				viewDirection);

		float normalDotLight =
			max(
				dot(
					normal,
					lightDirection),
				0.0);

		if (normalDotLight <= 0.0)
		{
			continue;
		}

		float normalDotHalfway =
			max(
				dot(
					normal,
					halfwayDirection),
				0.0);

		float halfwayDotView =
			max(
				dot(
					halfwayDirection,
					viewDirection),
				0.0001);

		float distribution =
			DistributionGGX(
				normal,
				halfwayDirection,
				uRoughness);

		float probabilityDensity =
			distribution *
			normalDotHalfway /
			(4.0 * halfwayDotView);

		float texelSolidAngle =
			4.0 *
			pi /
			(
				6.0 *
				uEnvironmentResolution *
				uEnvironmentResolution
			);

		float sampleSolidAngle =
			1.0 /
			max(
				float(sampleCount) *
				probabilityDensity,
				0.0001);

		float sourceMipLevel =
			0.5 *
			log2(
				sampleSolidAngle /
				texelSolidAngle);

		prefilteredColor +=
			textureLod(
				uEnvironmentMap,
				lightDirection,
				max(sourceMipLevel, 0.0)).rgb *
			normalDotLight;

		accumulatedWeight +=
			normalDotLight;
	}

	prefilteredColor /=
		max(
			accumulatedWeight,
			0.0001);

	fragColor =
		vec4(
			prefilteredColor,
			1.0);
}