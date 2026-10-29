#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTextureCoordinates;

uniform mat4 uModel;
uniform mat4 uLightViewProjection;

out vec2 vTextureCoordinates;

void main()
{
    vTextureCoordinates = aTextureCoordinates;
    gl_Position = uLightViewProjection * uModel * vec4(aPosition, 1.0);
}
