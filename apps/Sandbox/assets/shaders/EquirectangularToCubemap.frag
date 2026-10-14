#version 460 core

const vec2 inverseAtan =
	vec2(
		0.15915494309,
		0.31830988618);

in vec3 vLocalPosition;

uniform sampler2D uEquirectangularMap;

out vec4 fragColor;

vec2 DirectionToEquirectangularCoordinates(
	vec3 direction)
{
	vec2 coordinates =
		vec2(
			atan(direction.z, direction.x),
			asin(
				clamp(
					direction.y,
					-1.0,
					1.0)));

	coordinates *= inverseAtan;
	coordinates += 0.5;

	return coordinates;
}

void main()
{
	const vec3 direction =
		normalize(vLocalPosition);

	const vec2 textureCoordinates =
		DirectionToEquirectangularCoordinates(
			direction);

	const vec3 environmentColor =
		texture(
			uEquirectangularMap,
			textureCoordinates).rgb;

	fragColor =
		vec4(
			environmentColor,
			1.0);
}