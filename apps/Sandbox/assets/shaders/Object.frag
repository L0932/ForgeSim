#version 460 core

in vec3 vColor;
in vec2 vTextureCoordinates;

uniform vec4 uBaseColorFactor;
uniform sampler2D uBaseColorTexture;
uniform float uSelectionAmount;

out vec4 fragColor;

void main()
{
	vec4 sampledColor =
		texture(
			uBaseColorTexture,
			vTextureCoordinates);

	vec4 baseColor =
		sampledColor *
		uBaseColorFactor *
		vec4(vColor, 1.0);

	const vec3 selectionColor =
		vec3(1.0, 0.65, 0.1);

	vec3 finalColor =
		mix(
			baseColor.rgb,
			selectionColor,
			uSelectionAmount);

	fragColor =
		vec4(
			finalColor,
			baseColor.a);
}