#version 460 core

in vec2 vTextureCoordinates;

uniform sampler2D uBaseColorTexture;
uniform float uBaseColorAlpha;
uniform int uAlphaMaskEnabled;
uniform float uAlphaCutoff;

void main()
{
    if (uAlphaMaskEnabled != 0)
    {
        float alpha = texture(uBaseColorTexture, vTextureCoordinates).a *
            uBaseColorAlpha;

        if (alpha < uAlphaCutoff)
        {
            discard;
        }
    }
}
