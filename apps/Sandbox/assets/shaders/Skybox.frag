#version 460 core

in vec3 vDirection;

uniform samplerCube uEnvironmentMap;
uniform float uEnvironmentIntensity;
uniform bool uFlipEnvironmentVertically;

out vec4 fragColor;

void main()
{
    vec3 direction=normalize(vDirection);
    if (uFlipEnvironmentVertically) direction.y=-direction.y;
	vec3 environmentColor =
		textureLod(
			uEnvironmentMap,
			direction,
			0.0).rgb;

	fragColor =
		vec4(
			environmentColor *
			max(uEnvironmentIntensity, 0.0),
			1.0);
}