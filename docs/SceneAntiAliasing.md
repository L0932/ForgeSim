# Scene viewport anti-aliasing

## Implementation status

The first increment adds graphics-API-independent FXAA controls, validation,
and source texel-size calculation. It does not yet enable a GPU effect,
add UI controls, or change scene-document serialization.

## Planned rendering contract

The current tone-mapping shader applies exposure, tone maps linear HDR,
and explicitly encodes RGB as sRGB. Its destination is an ordinary RGBA8
texture, not an sRGB texture. The initial FXAA pass will consume those
display-encoded RGB values without automatic sRGB decoding, estimate luma
for edge detection, and write filtered display-encoded RGB. This is a
perceptual image filter, not another lighting calculation.

The intended Scene viewport order is:

1. Render the scene to linear HDR.
2. Apply exposure, tone mapping, and the existing linear-to-sRGB conversion.
3. If enabled, filter that display image into a separate color target.
4. Present the selected scene texture through ImGui, then draw editor UI.

There must remain exactly one linear-to-display conversion. FXAA must not
apply another gamma curve, and framebuffer sRGB conversion must be disabled
while writing its already encoded output. Reading and writing the same
texture in one draw is forbidden; use separate source and destination targets.
The pass's luma estimate will be an edge-detection signal from encoded RGB,
not physical linear-light luminance.

## Controls and dimensions

`FxaaSettings` defines dimensionless controls, each finite and in [0, 1]:

| Control | Initial value | Intended behavior |
| --- | --- | --- |
| Relative contrast threshold | 0.125 | Skip edges below a fraction of local maximum luma |
| Minimum contrast threshold | 0.03125 | Skip edges below an absolute contrast floor |
| Subpixel blending | 0.75 | Control additional smoothing of small-scale detail |

Lower thresholds process more low-contrast areas. Stronger subpixel blending
can reduce aliasing at the cost of softer detail. Zero subpixel blending
will disable that contribution, not the entire edge filter. Zero thresholds
are valid; the shader must still handle zero local contrast without division
by zero. Disabled settings are validated too, so re-enabling cannot expose
previously accepted NaN or out-of-range inputs. Validation rejects rather
than silently clamps bad values.

`CalculateFxaaTexelSize` returns `(1 / sourceWidth, 1 / sourceHeight)`.
Adding the first component to U moves one source pixel horizontally;
adding the second to V moves one source pixel vertically. Derive these
from the texture actually being sampled on every draw, including after
resizing. UI layout dimensions and desktop/window dimensions may differ.
A zero extent returns no value: the caller must skip rendering instead
of allocating a zero-size target or dividing by zero. Graphics-specific
texture-size limits remain the render target's responsibility.

## Headless validation

```bash
cmake --build out/build/x64-Debug --target ForgeSimFxaaSettingsTests --config Debug
ctest --test-dir out/build/x64-Debug -C Debug -R '^ForgeSim\.Renderer\.FxaaSettings$' --output-on-failure --no-tests=error
```

Tests cover finite/range validation, diagnostics, disabled controls, boundary
values, minimized sources, one-pixel sources, orientation changes, and large
dimension arithmetic. They require no window or OpenGL context. They do not
validate filtering quality; shader and visual checks belong to the GPU increment.

## Reading and limits

- [LearnOpenGL: Framebuffers](https://learnopengl.com/Advanced-OpenGL/Framebuffers)
  explains rendering a scene into a texture and sampling it in a fullscreen pass.
- [LearnOpenGL: Anti-aliasing](https://learnopengl.com/Advanced-OpenGL/Anti-aliasing)
  explains aliasing and MSAA; it is background, not an FXAA implementation guide.
- [NVIDIA FXAA white paper](https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf)
  explains contrast-based edge detection and subpixel filtering.

The planned filter uses the current color image only. It cannot recover lost
geometry detail or guarantee removal of temporal shimmer and specular aliasing.
TAA, motion vectors, upscaling, and MSAA are outside issue #70.
