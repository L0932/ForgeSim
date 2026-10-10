#version 460 core

in vec3 vColor;

uniform float uSelectionAmount;

out vec4 fragColor;

void main()
{
    const vec3 selectionColor =
        vec3(1.0, 0.65, 0.1);

    const vec3 finalColor =
        mix(
            vColor,
            selectionColor,
            uSelectionAmount);

    fragColor = vec4(finalColor, 1.0);
}