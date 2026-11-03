#version 460 core

in vec2 vTextureCoordinates;
out vec4 fragColor;
uniform sampler2D uSourceTexture;
uniform vec4 uTexelSize;
uniform int uEnabled;
uniform float uRelativeContrastThreshold;
uniform float uMinimumContrastThreshold;
uniform float uSubpixelBlending;

vec3 SampleColor(vec2 uv)
{
    return textureLod(uSourceTexture, uv, 0.0).rgb;
}

float Luma(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float SampleLuma(vec2 uv)
{
    return Luma(SampleColor(uv));
}

void main()
{
    vec2 uv = vTextureCoordinates;
    vec2 pixel = uTexelSize.xy;
    vec3 center = SampleColor(uv);
    if (uEnabled == 0)
    {
        fragColor = vec4(center, 1.0);
        return;
    }

    float m = Luma(center);
    float n = SampleLuma(uv + vec2(0.0, pixel.y));
    float s = SampleLuma(uv - vec2(0.0, pixel.y));
    float e = SampleLuma(uv + vec2(pixel.x, 0.0));
    float w = SampleLuma(uv - vec2(pixel.x, 0.0));
    float maximum = max(m, max(max(n, s), max(e, w)));
    float minimum = min(m, min(min(n, s), min(e, w)));
    float contrast = maximum - minimum;
    if (contrast <= 0.000001 ||
        contrast < max(uMinimumContrastThreshold, maximum * uRelativeContrastThreshold))
    {
        fragColor = vec4(center, 1.0);
        return;
    }

    float ne = SampleLuma(uv + pixel);
    float nw = SampleLuma(uv + vec2(-pixel.x, pixel.y));
    float se = SampleLuma(uv + vec2(pixel.x, -pixel.y));
    float sw = SampleLuma(uv - pixel);
    float horizontal = abs(nw + ne - 2.0 * n) +
        2.0 * abs(w + e - 2.0 * m) + abs(sw + se - 2.0 * s);
    float vertical = abs(nw + sw - 2.0 * w) +
        2.0 * abs(n + s - 2.0 * m) + abs(ne + se - 2.0 * e);
    bool horizontalEdge = horizontal >= vertical;
    float negative = horizontalEdge ? s : w;
    float positive = horizontalEdge ? n : e;
    float negativeGradient = abs(negative - m);
    float positiveGradient = abs(positive - m);
    bool negativeSteeper = negativeGradient >= positiveGradient;
    float edgeLuma = 0.5 * (m + (negativeSteeper ? negative : positive));
    vec2 perpendicular = horizontalEdge ? vec2(0.0, pixel.y) : vec2(pixel.x, 0.0);
    if (negativeSteeper) perpendicular = -perpendicular;
    vec2 alongEdge = horizontalEdge ? vec2(pixel.x, 0.0) : vec2(0.0, pixel.y);
    vec2 edgeUv = uv + 0.5 * perpendicular;

    const float distances[12] = float[12](
        1.0, 2.0, 3.5, 5.5, 7.5, 9.5, 11.5, 15.5, 23.5, 31.5, 39.5, 47.5);
    float negativeDistance = distances[11];
    float positiveDistance = distances[11];
    float negativeDelta = 0.0;
    float positiveDelta = 0.0;
    bool foundNegative = false;
    bool foundPositive = false;
    float threshold = max(0.25 * max(negativeGradient, positiveGradient), 0.000001);
    for (int index = 0; index < 12; ++index)
    {
        float distanceAlong = distances[index];
        if (!foundNegative)
        {
            negativeDelta = SampleLuma(edgeUv - alongEdge * distanceAlong) - edgeLuma;
            negativeDistance = distanceAlong;
            foundNegative = abs(negativeDelta) >= threshold;
        }
        if (!foundPositive)
        {
            positiveDelta = SampleLuma(edgeUv + alongEdge * distanceAlong) - edgeLuma;
            positiveDistance = distanceAlong;
            foundPositive = abs(positiveDelta) >= threshold;
        }
        if (foundNegative && foundPositive) break;
    }

    bool negativeCloser = negativeDistance <= positiveDistance;
    bool endpointFound = negativeCloser ? foundNegative : foundPositive;
    float endpointDelta = negativeCloser ? negativeDelta : positiveDelta;
    float edgeBlend = 0.0;
    if (endpointFound && ((endpointDelta < 0.0) != (m < edgeLuma)))
        edgeBlend = 0.5 - min(negativeDistance, positiveDistance) /
            (negativeDistance + positiveDistance);

    float average = (2.0 * (n + s + e + w) + ne + nw + se + sw) / 12.0;
    float subpixel = clamp(abs(average - m) / contrast, 0.0, 1.0);
    subpixel = subpixel * subpixel * (3.0 - 2.0 * subpixel);
    float blend = max(edgeBlend, subpixel * subpixel * uSubpixelBlending);
    fragColor = vec4(SampleColor(uv + perpendicular * blend), 1.0);
}
