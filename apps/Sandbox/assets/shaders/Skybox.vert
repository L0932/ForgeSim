#version 460 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vDirection;

void main()
{
	vDirection = aPosition;

	// Keep the camera's rotation but remove its translation so
	// the environment appears infinitely far away.
	mat4 rotationOnlyView =
		mat4(mat3(uView));

	vec4 clipPosition =
		uProjection *
		rotationOnlyView *
		vec4(aPosition, 1.0);

	// Place the skybox at the far depth plane.
	gl_Position =
		clipPosition.xyww;
}