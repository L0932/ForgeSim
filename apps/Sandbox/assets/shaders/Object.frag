#version 460 core

in vec3 vColor;
in vec2 vTextureCoordinates;

uniform sampler2D uBaseColorTexture;
uniform float uSelectionAmount;

out vec4 fragColor;

void main()
{
	vec4 sampledColor =
		texture(
			uBaseColorTexture,
			vTextureCoordinates);

	vec3 baseColor =
		sampledColor.rgb *
		vColor;

	vec3 selectionColor =
		vec3(1.0, 0.65, 0.1);

	vec3 finalColor =
		mix(
			baseColor,
			selectionColor,
			uSelectionAmount);

	fragColor =
		vec4(
			finalColor,
			sampledColor.a);
}