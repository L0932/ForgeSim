#version 460 core

in vec2 vTextureCoordinates;
layout(location = 0) out vec4 oTransmission;

uniform sampler2D uBaseColorTexture;
uniform vec3 uBaseColorUvRow0;
uniform vec3 uBaseColorUvRow1;
uniform float uBaseColorAlpha;
uniform int uAlphaMaskEnabled;
uniform float uAlphaCutoff;
uniform vec3 uTransmissionTint;
uniform int uTransmissionUsesTexture;

void main()
{
    vec4 texel = vec4(1.0);
    if (uAlphaMaskEnabled != 0 || uTransmissionUsesTexture != 0)
    {
        vec3 coordinate = vec3(vTextureCoordinates, 1.0);
        vec2 uv = vec2(dot(uBaseColorUvRow0, coordinate), dot(uBaseColorUvRow1, coordinate));
        texel = texture(uBaseColorTexture, uv);
    }

    if (uAlphaMaskEnabled != 0 && texel.a * uBaseColorAlpha < uAlphaCutoff)
        discard;

    vec3 tint = uTransmissionTint * (uTransmissionUsesTexture != 0 ? texel.rgb : vec3(1.0));
    oTransmission = vec4(tint, gl_FragCoord.z);
}
