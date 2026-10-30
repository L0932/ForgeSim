#version 460 core

const float pi = 3.14159265359;

in vec3 vColor;
in vec2 vTextureCoordinates;
in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec4 vWorldTangent;

uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

uniform int uAlphaMaskEnabled;
uniform float uAlphaCutoff;

uniform int uDoubleSided;

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

uniform sampler2D uDirectionalShadowMap;
uniform mat4 uDirectionalLightViewProjection;
uniform int uDirectionalShadowsEnabled;

uniform int uDirectionalShadowFilteringEnabled;
uniform float uDirectionalShadowMinimumBias;
uniform float uDirectionalShadowSlopeBias;

const int maximumPointLights = 16;

struct PointLightData
{
	vec3 position;
	vec3 radiance;
	float range;
};

uniform int uPointLightCount;
uniform PointLightData uPointLights[maximumPointLights];

const int maximumSpotLights = 16;

struct SpotLightData
{
	vec3 position;
	vec3 emissionDirection;
	vec3 radiance;
	float range;
	float innerConeCos;
	float outerConeCos;
};

uniform int uSpotLightCount;
uniform SpotLightData uSpotLights[maximumSpotLights];

uniform float uEnvironmentIntensity;
uniform samplerCube uDiffuseIrradianceMap;
uniform samplerCube uPrefilteredSpecularMap;
uniform sampler2D uBrdfLookupTexture;
uniform float uMaximumSpecularLod;
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

vec3 FresnelSchlickRoughness(
	float cosineTheta,
	vec3 reflectanceAtNormalIncidence,
	float roughness)
{
	return
		reflectanceAtNormalIncidence +
		(
			max(
				vec3(1.0 - roughness),
				reflectanceAtNormalIncidence) -
			reflectanceAtNormalIncidence
		) *
		pow(
			clamp(
				1.0 - cosineTheta,
				0.0,
				1.0),
			5.0);
}

vec3 EvaluateDirectLighting(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	vec3 radiance,
	vec3 baseColor,
	float metallic,
	float roughness,
	vec3 reflectanceAtNormalIncidence)
{
	float normalDotView =
		max(dot(normal, viewDirection), 0.0);

	float normalDotLight =
		max(dot(normal, lightDirection), 0.0);

	if (normalDotView <= 0.0 || normalDotLight <= 0.0)
	{
		return vec3(0.0);
	}

	vec3 halfwayDirection =
		normalize(viewDirection + lightDirection);

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
				dot(halfwayDirection, viewDirection),
				0.0),
			reflectanceAtNormalIncidence);

	float denominator =
		max(
			4.0 * normalDotView * normalDotLight,
			0.0001);

	vec3 specular =
		normalDistribution *
		geometry *
		fresnel /
		denominator;

	vec3 diffuseFraction =
		(vec3(1.0) - fresnel) *
		(1.0 - metallic);

	return
		(
			diffuseFraction * baseColor / pi +
			specular
		) *
		radiance *
		normalDotLight;
}

float DistanceAttenuation(
	float distanceSquared,
	float range)
{
	float distanceToLight = sqrt(distanceSquared);
	float normalizedDistance = distanceToLight / range;

	float normalizedDistanceSquared =
		normalizedDistance * normalizedDistance;

	float rangeFade =
		max(
			1.0 -
				normalizedDistanceSquared *
				normalizedDistanceSquared,
			0.0);

	// Artistic softening radius, measured in world units.
	const float softeningDistance = 1.0;
	const float softeningDistanceSquared =
		softeningDistance * softeningDistance;

	return
		rangeFade * rangeFade *
		softeningDistanceSquared /
		(distanceSquared + softeningDistanceSquared);
}

float FilteredShadowComparison(
    vec2 uv,
    float receiverDepth,
    vec2 depthGradient)
{
    vec2 mapSize =
        vec2(textureSize(uDirectionalShadowMap, 0));

    // Express the position relative to texel centers.
    vec2 texelPosition = uv * mapSize - 0.5;
    vec2 baseTexel = floor(texelPosition);
    vec2 weight = fract(texelPosition);

    vec2 uv00 = (baseTexel + vec2(0.5)) / mapSize;
    vec2 texelSize = 1.0 / mapSize;

	float depth00 =
    texture(uDirectionalShadowMap, uv00).r;

	float depth10 =
		texture(
			uDirectionalShadowMap,
			uv00 + vec2(texelSize.x, 0.0)).r;

	float depth01 =
		texture(
			uDirectionalShadowMap,
			uv00 + vec2(0.0, texelSize.y)).r;

	float depth11 =
		texture(
			uDirectionalShadowMap,
			uv00 + texelSize).r;

	float depthAt00 =
		receiverDepth +
		dot(depthGradient, uv00 - uv);

	float depthStepX = depthGradient.x * texelSize.x;
	float depthStepY = depthGradient.y * texelSize.y;

	float visibility00 =
		depthAt00 <= depth00 ? 1.0 : 0.0;

	float visibility10 =
		depthAt00 + depthStepX <= depth10 ? 1.0 : 0.0;

	float visibility01 =
		depthAt00 + depthStepY <= depth01 ? 1.0 : 0.0;

	float visibility11 =
		depthAt00 + depthStepX + depthStepY <= depth11
			? 1.0 : 0.0;

    return mix(
        mix(visibility00, visibility10, weight.x),
        mix(visibility01, visibility11, weight.x),
        weight.y);
}

float DirectionalLightVisibility(vec3 geometricNormal)
{
    if (uDirectionalShadowsEnabled == 0)
    {
        return 1.0;
    }

    vec4 lightClipPosition =
        uDirectionalLightViewProjection *
        vec4(vWorldPosition, 1.0);

    if (lightClipPosition.w <= 0.0)
    {
        return 1.0;
    }

    vec3 shadowPosition =
        lightClipPosition.xyz / lightClipPosition.w;

    shadowPosition = shadowPosition * 0.5 + 0.5;

	vec3 shadowDx = dFdx(shadowPosition);
	vec3 shadowDy = dFdy(shadowPosition);

	float determinant =
		shadowDx.x * shadowDy.y -
		shadowDx.y * shadowDy.x;

	float derivativeScale =
		length(shadowDx.xy) * length(shadowDy.xy);

	vec2 depthGradient = vec2(0.0);

	if (abs(determinant) >
		0.000001 * max(derivativeScale, 1e-20))
	{
		depthGradient = vec2(
			shadowDy.y * shadowDx.z -
				shadowDx.y * shadowDy.z,

			shadowDx.x * shadowDy.z -
				shadowDy.x * shadowDx.z
		) / determinant;
	}

    if (any(lessThan(shadowPosition, vec3(0.0))) ||
        any(greaterThan(shadowPosition, vec3(1.0))))
    {
        return 1.0;
    }

    float normalDotLight =
    clamp(
        dot(
            normalize(geometricNormal),
            normalize(uLightDirection)),
        0.0,
        1.0);

	float bias =
		max(
			uDirectionalShadowMinimumBias,
			uDirectionalShadowSlopeBias *
				(1.0 - normalDotLight));

	float receiverDepth = shadowPosition.z - bias;

	if (uDirectionalShadowFilteringEnabled == 0)
	{
		float storedDepth =
			texture(
				uDirectionalShadowMap,
				shadowPosition.xy).r;

		return receiverDepth <= storedDepth ? 1.0 : 0.0;
	}

	vec2 texelSize =
		1.0 / vec2(textureSize(uDirectionalShadowMap, 0));

	float visibility = 0.0;

	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			vec2 offset = vec2(x, y) * texelSize;

			float sampleReceiverDepth =
				receiverDepth + dot(depthGradient, offset);

			visibility += FilteredShadowComparison(
				shadowPosition.xy + offset,
				sampleReceiverDepth,
				depthGradient);
		}
	}

	return visibility / 9.0;
}

void main()
{

	vec3 shadowNormal = normalize(vWorldNormal);

	if (uDoubleSided != 0 && !gl_FrontFacing)
	{
		shadowNormal = -shadowNormal;
	}

	float directionalVisibility =
		DirectionalLightVisibility(shadowNormal);

	vec4 sampledBaseColor =
		texture(
			uBaseColorTexture,
			vTextureCoordinates) *
		uBaseColorFactor *
		vec4(vColor, 1.0);

	if (uAlphaMaskEnabled != 0 &&
		sampledBaseColor.a < uAlphaCutoff)
	{
		discard;
	}

	vec3 geometricNormal =
		normalize(vWorldNormal);

	if (uDoubleSided != 0 &&
		!gl_FrontFacing)
	{
		geometricNormal =
			-geometricNormal;
	}

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

	// Also used by the environment-lighting calculations below.
	float normalDotView =
		max(dot(normal, viewDirection), 0.0);

	vec3 directLighting =
		EvaluateDirectLighting(
			normal,
			viewDirection,
			normalize(uLightDirection),
			uLightRadiance,
			sampledBaseColor.rgb,
			metallic,
			roughness,
			reflectanceAtNormalIncidence);

	directLighting *= directionalVisibility;

	for (int index = 0; index < uPointLightCount; ++index)
	{
		PointLightData light = uPointLights[index];

		// Surface-to-light vector, matching the BRDF convention.
		vec3 toLight =
			light.position - vWorldPosition;

		float distanceSquared = dot(toLight, toLight);

		// Exactly at the light, the direction is undefined.
		if (distanceSquared <= 1e-12 ||
			sqrt(distanceSquared) >= light.range)
		{
			continue;
		}

		vec3 lightDirection =
			toLight * inversesqrt(distanceSquared);

		float attenuation =
			DistanceAttenuation(
				distanceSquared,
				light.range);

		vec3 incomingRadiance =
			light.radiance * attenuation;

		directLighting +=
			EvaluateDirectLighting(
				normal,
				viewDirection,
				lightDirection,
				incomingRadiance,
				sampledBaseColor.rgb,
				metallic,
				roughness,
				reflectanceAtNormalIncidence);
	}

	for (int index = 0; index < uSpotLightCount; ++index)
	{
		SpotLightData light = uSpotLights[index];

		vec3 toLight =
			light.position - vWorldPosition;

		float distanceSquared =
			dot(toLight, toLight);

		if (distanceSquared <= 1e-12 ||
			sqrt(distanceSquared) >= light.range)
		{
			continue;
		}

		// Surface toward the light.
		vec3 lightDirection =
			toLight * inversesqrt(distanceSquared);

		// Compare emission with the opposite direction:
		// light toward the surface.
		float coneCos =
			dot(light.emissionDirection, -lightDirection);

		float coneAttenuation =
			smoothstep(
				light.outerConeCos,
				light.innerConeCos,
				coneCos);

		if (coneAttenuation <= 0.0)
		{
			continue;
		}

		float distanceAttenuation =
			DistanceAttenuation(
				distanceSquared,
				light.range);

		vec3 incomingRadiance =
			light.radiance *
			distanceAttenuation *
			coneAttenuation;

		directLighting +=
			EvaluateDirectLighting(
				normal,
				viewDirection,
				lightDirection,
				incomingRadiance,
				sampledBaseColor.rgb,
				metallic,
				roughness,
				reflectanceAtNormalIncidence);
	}

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

	vec3 environmentFresnel =
		FresnelSchlickRoughness(
			normalDotView,
			reflectanceAtNormalIncidence,
			roughness);

	vec3 environmentDiffuseFraction =
		(vec3(1.0) - environmentFresnel) *
		(1.0 - metallic);

	vec3 irradiance =
		texture(
			uDiffuseIrradianceMap,
			normal).rgb * 
			uEnvironmentIntensity;

	vec3 indirectDiffuseLighting =
		environmentDiffuseFraction *
		irradiance *
		sampledBaseColor.rgb *
		ambientOcclusion;

	vec3 emissiveLighting =
		texture(
			uEmissiveTexture,
			vTextureCoordinates).rgb *
		uEmissiveFactor;

	vec3 reflectionDirection =
	reflect(
		-viewDirection,
		normal);

	vec3 prefilteredEnvironment =
		textureLod(
			uPrefilteredSpecularMap,
			reflectionDirection,
			roughness *
				uMaximumSpecularLod).rgb *
		uEnvironmentIntensity;

	vec2 integratedBrdf =
		texture(
			uBrdfLookupTexture,
			vec2(
				normalDotView,
				roughness)).rg;

	vec3 indirectSpecularLighting =
		prefilteredEnvironment *
		(
			environmentFresnel *
				integratedBrdf.x +
			integratedBrdf.y
		) *
		ambientOcclusion;

	vec3 surfaceColor =
		indirectDiffuseLighting +
		indirectSpecularLighting +
		directLighting +
		emissiveLighting;

	vec3 finalColor =
		mix(
			surfaceColor,
			uSelectionColor,
			uSelectionAmount);

	fragColor = vec4(
		finalColor,
		sampledBaseColor.a);
}