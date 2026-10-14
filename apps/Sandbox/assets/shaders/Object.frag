#version 460 core

const float pi = 3.14159265359;

in vec3 vColor;
in vec2 vTextureCoordinates;
in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec4 vWorldTangent;

uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

uniform sampler2D uMetallicRoughnessTexture;
uniform float uMetallicFactor;

uniform sampler2D uNormalTexture;
uniform float uNormalScale;

uniform sampler2D uOcclusionTexture;
uniform float uOcclusionStrength;

uniform sampler2D uEmissiveTexture;
uniform vec3 uEmissiveFactor;

uniform vec3 uSelectionColor;
uniform float uSelectionAmount;

uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightRadiance;

uniform float uRoughnessFactor;

out vec4 fragColor;

float DistributionGGX(
	vec3 normal,
	vec3 halfwayDirection,
	float roughness)
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;

	float normalDotHalfway =
		max(dot(normal, halfwayDirection), 0.0);

	float normalDotHalfwaySquared =
		normalDotHalfway * normalDotHalfway;

	float denominator =
		normalDotHalfwaySquared *
		(alphaSquared - 1.0) +
		1.0;

	return alphaSquared /
		max(
			pi * denominator * denominator,
			0.0001);
}

float GeometrySchlickGGX(
	float normalDotDirection,
	float roughness)
{
	float roughnessPlusOne =
		roughness + 1.0;

	float k =
		(roughnessPlusOne * roughnessPlusOne) /
		8.0;

	return normalDotDirection /
		max(
			normalDotDirection * (1.0 - k) + k,
			0.0001);
}

float GeometrySmith(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	float roughness)
{
	float normalDotView =
		max(dot(normal, viewDirection), 0.0);

	float normalDotLight =
		max(dot(normal, lightDirection), 0.0);

	return
		GeometrySchlickGGX(
			normalDotView,
			roughness) *
		GeometrySchlickGGX(
			normalDotLight,
			roughness);
}

vec3 FresnelSchlick(
	float cosineTheta,
	vec3 reflectanceAtNormalIncidence)
{
	return reflectanceAtNormalIncidence +
		(1.0 - reflectanceAtNormalIncidence) *
		pow(
			clamp(1.0 - cosineTheta, 0.0, 1.0),
			5.0);
}

vec3 SrgbToLinear(vec3 color)
{
	bvec3 useLinearSegment =
		lessThanEqual(
			color,
			vec3(0.04045));

	vec3 linearSegment =
		color / 12.92;

	vec3 exponentialSegment =
		pow(
			(color + 0.055) / 1.055,
			vec3(2.4));

	return mix(
		exponentialSegment,
		linearSegment,
		useLinearSegment);
}

vec3 LinearToSrgb(vec3 color)
{
	color = max(color, vec3(0.0));

	bvec3 useLinearSegment =
		lessThanEqual(
			color,
			vec3(0.0031308));

	vec3 linearSegment =
		color * 12.92;

	vec3 exponentialSegment =
		1.055 *
		pow(
			color,
			vec3(1.0 / 2.4)) -
		0.055;

	return mix(
		exponentialSegment,
		linearSegment,
		useLinearSegment);
}

vec3 ToneMapAces(vec3 color)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;

	return clamp(
		(color * (a * color + b)) /
		(color * (c * color + d) + e),
		0.0,
		1.0);
}

void main()
{
	vec4 baseColorSample =
		texture(
			uBaseColorTexture,
			vTextureCoordinates);

	vec4 sampledBaseColor =
		vec4(
			SrgbToLinear(baseColorSample.rgb),
			baseColorSample.a) *
		uBaseColorFactor *
		vec4(vColor, 1.0);

	vec3 geometricNormal =
		normalize(vWorldNormal);

	vec3 projectedTangent =
		vWorldTangent.xyz -
		geometricNormal *
		dot(
			vWorldTangent.xyz,
			geometricNormal);

	vec3 normal = geometricNormal;

	float tangentLengthSquared =
		dot(
			projectedTangent,
			projectedTangent);

	if (tangentLengthSquared > 0.000001)
	{
		vec3 tangent =
			projectedTangent *
			inversesqrt(tangentLengthSquared);

		vec3 bitangent =
			normalize(
				cross(
					geometricNormal,
					tangent)) *
				vWorldTangent.w;

		vec3 tangentSpaceNormal =
			texture(
				uNormalTexture,
				vTextureCoordinates).xyz *
			2.0 -
			1.0;

		tangentSpaceNormal.xy *= uNormalScale;

		mat3 tangentToWorld =
			mat3(
				tangent,
				bitangent,
				geometricNormal);

		normal =
			normalize(
				tangentToWorld *
				tangentSpaceNormal);
	}

	vec3 viewDirection =
		normalize(
			uCameraPosition -
			vWorldPosition);

	vec3 lightDirection =
		normalize(uLightDirection);

	vec3 halfwayDirection =
		normalize(
			viewDirection +
			lightDirection);

	vec4 sampledMetallicRoughness =
		texture(
			uMetallicRoughnessTexture,
			vTextureCoordinates);

	// glTF stores roughness in green and metallic in blue.
	float metallic =
		clamp(
			uMetallicFactor *
			sampledMetallicRoughness.b,
			0.0,
			1.0);

	float roughness =
		clamp(
			uRoughnessFactor *
			sampledMetallicRoughness.g,
			0.045,
			1.0);

	vec3 reflectanceAtNormalIncidence =
		mix(
			vec3(0.04),
			sampledBaseColor.rgb,
			metallic);

	float normalDistribution =
		DistributionGGX(
			normal,
			halfwayDirection,
			roughness);

	float geometry =
		GeometrySmith(
			normal,
			viewDirection,
			lightDirection,
			roughness);

	vec3 fresnel =
		FresnelSchlick(
			max(
				dot(
					halfwayDirection,
					viewDirection),
				0.0),
			reflectanceAtNormalIncidence);

	vec3 specular =
		normalDistribution *
		geometry *
		fresnel;

	float normalDotView =
		max(
			dot(normal, viewDirection),
			0.0);

	float normalDotLight =
		max(
			dot(normal, lightDirection),
			0.0);

	float denominator =
		max(
			4.0 *
			normalDotView *
			normalDotLight,
			0.0001);

	specular /= denominator;

	vec3 reflectedFraction = fresnel;

	vec3 diffuseFraction =
		(vec3(1.0) - reflectedFraction) *
		(1.0 - metallic);

	vec3 directLighting =
		(
			diffuseFraction *
			sampledBaseColor.rgb /
			pi +
			specular
		) *
		uLightRadiance *
		normalDotLight;

	float sampledOcclusion =
		texture(
			uOcclusionTexture,
			vTextureCoordinates).r;

	float ambientOcclusion =
		mix(
			1.0,
			sampledOcclusion,
			clamp(
				uOcclusionStrength,
				0.0,
				1.0));

	// Temporary ambient term until image-based lighting is added.
	vec3 ambientLighting =
		0.03 *
		sampledBaseColor.rgb *
		ambientOcclusion;

	vec3 emissiveLighting =
		SrgbToLinear(
			texture(
				uEmissiveTexture,
				vTextureCoordinates).rgb) *
		uEmissiveFactor;

	vec3 surfaceColor =
		ambientLighting +
		directLighting +
		emissiveLighting;

	vec3 finalColor =
		mix(
			surfaceColor,
			uSelectionColor,
			uSelectionAmount);

	vec3 toneMappedColor =
		ToneMapAces(finalColor);

	fragColor = vec4(
		LinearToSrgb(toneMappedColor),
		sampledBaseColor.a);
}