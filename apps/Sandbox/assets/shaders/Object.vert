#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTextureCoordinates;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec4 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vColor;
out vec2 vTextureCoordinates;
out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec4 vWorldTangent;

void main()
{
    vec4 worldPosition =
        uModel * vec4(aPosition, 1.0);

    mat3 normalMatrix =
        transpose(inverse(mat3(uModel)));

    vColor = aColor;
    vTextureCoordinates = aTextureCoordinates;
    vWorldPosition = worldPosition.xyz;
    vWorldNormal =
        normalize(normalMatrix * aNormal);

    vWorldTangent = vec4(
        normalize(normalMatrix * aTangent.xyz),
        aTangent.w);

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}