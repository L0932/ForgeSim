#version 460 core

in vec2 vTextureCoordinates;

uniform sampler2D uHdrTexture;
uniform float uExposure;

out vec4 fragColor;

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

vec3 LinearToSrgb(vec3 color)
{
	const float threshold = 0.0031308;

	vec3 lower =
		12.92 * color;

	vec3 upper =
		1.055 *
		pow(
			color,
			vec3(1.0 / 2.4)) -
		0.055;

	return mix(
		lower,
		upper,
		step(
			vec3(threshold),
			color));
}

void main()
{
	vec3 hdrColor =
		texture(
			uHdrTexture,
			vTextureCoordinates).rgb;

	vec3 exposedColor =
		hdrColor *
		max(uExposure, 0.0);

	vec3 toneMappedColor =
		ToneMapAces(exposedColor);

	fragColor =
		vec4(
			LinearToSrgb(toneMappedColor),
			1.0);
}