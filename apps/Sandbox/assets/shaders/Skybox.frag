#version 460 core

in vec3 vDirection;

uniform samplerCube uEnvironmentMap;
uniform float uEnvironmentIntensity;

out vec4 fragColor;

void main()
{
	vec3 environmentColor =
		textureLod(
			uEnvironmentMap,
			normalize(vDirection),
			0.0).rgb;

	fragColor =
		vec4(
			environmentColor *
			max(uEnvironmentIntensity, 0.0),
			1.0);
}