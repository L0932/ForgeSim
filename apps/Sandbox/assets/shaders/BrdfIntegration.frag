#version 460 core

const float pi = 3.14159265359;
const uint sampleCount = 512u;

in vec2 vTextureCoordinates;

layout(location = 0) out vec2 fragColor;

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

	return float(bits) *
		2.3283064365386963e-10;
}

vec2 Hammersley(
	uint sampleIndex,
	uint numberOfSamples)
{
	return vec2(
		float(sampleIndex) /
			float(numberOfSamples),
		RadicalInverseVanDerCorput(
			sampleIndex));
}

vec3 ImportanceSampleGGX(
	vec2 samplePoint,
	vec3 normal,
	float roughness)
{
	float alpha =
		roughness *
		roughness;

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

float GeometrySchlickGGX(
	float normalDotDirection,
	float roughness)
{
	float alpha =
		roughness *
		roughness;

	float k =
		alpha /
		2.0;

	return normalDotDirection /
		max(
			normalDotDirection *
				(1.0 - k) +
				k,
			0.0001);
}

float GeometrySmith(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	float roughness)
{
	float normalDotView =
		max(
			dot(
				normal,
				viewDirection),
			0.0);

	float normalDotLight =
		max(
			dot(
				normal,
				lightDirection),
			0.0);

	return
		GeometrySchlickGGX(
			normalDotView,
			roughness) *
		GeometrySchlickGGX(
			normalDotLight,
			roughness);
}

vec2 IntegrateBrdf(
	float normalDotView,
	float roughness)
{
	vec3 normal =
		vec3(0.0, 0.0, 1.0);

	vec3 viewDirection = vec3(
		sqrt(
			max(
				1.0 -
					normalDotView *
					normalDotView,
				0.0)),
		0.0,
		normalDotView);

	float coefficientA = 0.0;
	float coefficientB = 0.0;

	for (uint sampleIndex = 0u;
		sampleIndex < sampleCount;
		++sampleIndex)
	{
		const vec2 samplePoint =
			Hammersley(
				sampleIndex,
				sampleCount);

		const vec3 halfwayDirection =
			ImportanceSampleGGX(
				samplePoint,
				normal,
				roughness);

		const vec3 lightDirection =
			normalize(
				2.0 *
					dot(
						viewDirection,
						halfwayDirection) *
					halfwayDirection -
				viewDirection);

		const float normalDotLight =
			max(
				lightDirection.z,
				0.0);

		const float normalDotHalfway =
			max(
				halfwayDirection.z,
				0.0);

		const float viewDotHalfway =
			max(
				dot(
					viewDirection,
					halfwayDirection),
				0.0);

		if (normalDotLight > 0.0)
		{
			const float geometry =
				GeometrySmith(
					normal,
					viewDirection,
					lightDirection,
					roughness);

			const float geometryVisibility =
				geometry *
				viewDotHalfway /
				max(
					normalDotHalfway *
						normalDotView,
					0.0001);

			const float fresnel =
				pow(
					1.0 -
						viewDotHalfway,
					5.0);

			coefficientA +=
				(1.0 - fresnel) *
				geometryVisibility;

			coefficientB +=
				fresnel *
				geometryVisibility;
		}
	}

	return vec2(
		coefficientA,
		coefficientB) /
		float(sampleCount);
}

void main()
{
	float normalDotView =
		clamp(
			vTextureCoordinates.x,
			0.001,
			1.0);

	float roughness =
		clamp(
			vTextureCoordinates.y,
			0.0,
			1.0);

	fragColor =
		IntegrateBrdf(
			normalDotView,
			roughness);
}