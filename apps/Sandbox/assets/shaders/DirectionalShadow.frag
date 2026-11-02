#version 460 core

in vec2 vTextureCoordinates;

uniform sampler2D uBaseColorTexture;
uniform vec3 uBaseColorUvRow0;
uniform vec3 uBaseColorUvRow1;
uniform float uBaseColorAlpha;
uniform int uAlphaMaskEnabled;
uniform float uAlphaCutoff;

void main()
{
    if (uAlphaMaskEnabled != 0)
    {
        vec3 coordinate = vec3(vTextureCoordinates, 1.0);
        vec2 uv = vec2(dot(uBaseColorUvRow0, coordinate), dot(uBaseColorUvRow1, coordinate));
        float alpha = texture(uBaseColorTexture, uv).a *
            uBaseColorAlpha;

        if (alpha < uAlphaCutoff)
        {
            discard;
        }
    }
}
