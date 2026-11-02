# glTF texture transforms

## Implementation stage

Import and rendering now preserve and apply per-texture transforms. The visible
object pass and alpha-masked directional-shadow pass use the same base-color UV
mapping. Other material texture usages keep their own independent mappings.

Only `KHR_texture_transform` is currently accepted in `extensionsRequired`.
Unknown required extensions fail with the extension name and asset path, in both
strict and texture-fallback modes. Optional unsupported extensions continue using
their authored core glTF fallback; that does not reproduce the extension's effect.

## Where the data belongs

`MaterialTextureReference` now includes a `TextureTransform` with:

- `offset`: two values, default `(0, 0)`.
- `rotationRadians`: one angle, default `0`.
- `scale`: two values, default `(1, 1)`.

Each base-color, metallic-roughness, normal, occlusion, and emissive reference
owns its transform. They may use the same image with different mappings.
Mesh vertex positions, UVs, indices, and authored node transforms are unchanged.

The glTF texture's `texCoord` selects a UV set. An explicit `texCoord` inside
`KHR_texture_transform` overrides that selection, including an explicit zero.
The importer stores the effective selection in `textureCoordinateSet`.

Fallback filtering uses the same effective selection. For example, base set 1
with an extension override of 0 can use `TEXCOORD_0`; base set 0 with an override
of 1 is skipped in fallback mode. Only primary UVs are currently rendered.
Strict import continues preserving positive set indices for the renderer to
reject if unsupported, while rejecting negative effective indices immediately.
This patch does not add additional mesh UV channels.

## Transform order

For a coordinate `(u, v)`, let `(sx, sy)` be scale, `(ox, oy)` be offset, and
`r` be rotation in radians. The operation is:

```text
scaledU = sx * u
scaledV = sy * v
u' = ox + cos(r) * scaledU - sin(r) * scaledV
v' = oy + sin(r) * scaledU + cos(r) * scaledV
```

In homogeneous column-vector notation, this is `T * R * S * (u, v, 1)`.
Scale happens first, rotation around the UV origin second, and translation last.
Changing the order changes the mapping. Rotation is defined in glTF's UV
coordinate convention; do not introduce an extra V flip here.

The CPU reference function `ApplyTextureTransform` implements this operation.
It does not wrap or clamp the result. Texture samplers apply repeat, mirrored
repeat, or clamp behavior after the sampling coordinates have been calculated.

For the cathedral window, a scale of `(7, 7)` turns `(0.2, 0.3)` into
`(1.4, 2.1)` before sampler wrapping. With repeat wrapping, the pattern repeats
more often and its colored pieces appear smaller. Ignoring that scale produces
the observed enlarged pattern.

## Validation and limitations

Imported offset, rotation, and scale must be finite. Negative scale is legal
and mirrors the mapping; zero scale is also retained rather than rejected or
silently replaced. Very large finite values are not clamped. Callers of the CPU
helper are responsible for supplying valid coordinates and a finite transform;
extreme arithmetic can still exceed floating-point range.

Malformed transform values remain errors in both strict and fallback modes.
Texture fallback is limited to unusable UV selections; it does not suppress
invalid transform data. Normal-map strength and occlusion strength remain
separate properties from UV scale.

## GPU data flow

1. The importer stores a transform on each `MaterialTextureReference`.
2. When resolving a GPU texture binding, `CalculateTextureTransformRows`
   precomputes two affine rows: `(cos(r)*sx, -sin(r)*sy, ox)` and
   `(sin(r)*sx, cos(r)*sy, oy)`.
3. The GPU binding retains those rows alongside its texture and sampler pointers.
   Texture images can still be shared by bindings with different transforms.
4. `BindMaterial` uploads both rows for every usage on every draw, including
   identity rows for missing textures. This avoids stale transforms leaking from
   a previously drawn material.
5. The fragment shader calculates each UV component with a dot product against
   `(u, v, 1)`, then samples the corresponding texture. Trigonometry is performed
   once when the CPU binding is created, rather than per fragment.
6. Directional-shadow alpha testing uploads the same base-color rows, ensuring
   holes in a masked surface line up with its shadow.

The sampler wrapping and color-space interpretation are unchanged. Emissive and
base-color textures remain sRGB inputs; normal, metallic-roughness, and occlusion
textures remain linear data.

## Normal-map basis

Normal-map RGB stores a direction in a local tangent frame. For an unchanged
linear mapping, including offset-only transforms, the renderer preserves the
existing authored/generated tangent frame.

When scale, rotation, or reflection changes that mapping, the fragment shader
uses screen-space derivatives of world position and transformed normal-map UVs
to reconstruct the directions of increasing U and V. It projects and normalizes
the tangent against the interpolated surface normal, then determines bitangent
handedness from the reconstructed V direction. Mirroring U and mirroring V must
produce different direction changes.

All derivatives and the normal sample are evaluated before alpha discard or
non-uniform control flow. Common derivative magnitudes are normalized before
checking degeneracy, so the guard is not tied to camera distance or world units.
Singular mappings (including zero scale), degenerate UV gradients, or unusable
frames use the geometric normal. This avoids divisions by zero and undefined
normalization. Normal strength remains independent of UV scale.

The derivative frame is calculated per fragment, rather than re-baking tangent
attributes. Results can differ from artist-authored tangent frames at seams or
under strong distortion; the original frame is preserved for identity mappings.

## Headless tests

The original `TextureTransforms.gltf` fixture uses embedded triangle data and a
one-pixel image, with independent transforms on all five texture usages. Tests
also write temporary variants to exercise defaults, UV-set overrides, shared
material fallback, and non-finite values represented by out-of-range JSON numbers.
No purchased cathedral content is required or distributed.

```bash
cmake --build out/build/x64-Debug --target ForgeSimTextureTransformTests ForgeSimGltfLoaderTests ForgeSimMaterialDataTests --config Debug
ctest --test-dir out/build/x64-Debug -C Debug -R '^ForgeSim\.Assets\.(TextureTransform|GltfLoader|MaterialData)$' --output-on-failure --no-tests=error
```

Repeat with `out/build/x64-Release` and configuration `Release` as appropriate.

## Reading

- [Khronos KHR_texture_transform specification](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md): defaults, transform order, UV-set override, and coordinate convention.
- [LearnOpenGL: Getting Started / Textures](https://learnopengl.com/Getting-started/Textures): focus on texture coordinates, sampling, and wrapping.
- [LearnOpenGL: Getting Started / Transformations](https://learnopengl.com/Getting-started/Transformations): focus on matrix composition and why order matters.
- [LearnOpenGL: Advanced Lighting / Normal Mapping](https://learnopengl.com/Advanced-Lighting/Normal-Mapping): preparation for the later tangent-space integration.


## Visual test model

From the repository root after building Sandbox:

```bash
"./out/build/x64-Debug/apps/Sandbox/ForgeSimSandbox.exe" TextureTransformTest.gltf
```

The model has ten front-facing square panels and a dark receiver behind them.
Look toward the front (positive Z side), disable editor overlays for captures,
and use a directional light whose X, Y, and Z components are all nonzero. For
example, direction-to-light `(0.4, 0.7, 1)` gets normalized by the editor. Turn off
point/spot lights and use low environment intensity when judging normal response.
Keep exposure and camera fixed for comparisons.

| Row, left to right | Expected behavior |
| --- | --- |
| A: emissive pattern | Identity; 7x repetition; 90-degree rotation; U mirror |
| B: constant tilted normal | Identity tilts toward +U; rotation changes the tilt direction; U mirror reverses it; zero UV scale uses the geometric normal |
| C: two alpha-mask panels | Identity cutout; scaled/rotated/offset cutout |

The asymmetric white marker makes orientation visible. Row A's emission makes
its patterns readable independently of the light. Row B deliberately samples a
constant tilted normal: its lighting changes should come from the tangent frame,
not from looking up different pixels in the map. Enable directional shadows and
adjust the light direction to inspect row C's cutout shadows on the receiver.
They must follow the same holes as the visible surfaces. Shadow-map resolution
and filtering can still soften or alias their edges; this is not anti-aliasing.

Also compare the cathedral with its authored Mapping values unchanged. The 7x
window pattern should become smaller/more repeated; wall, wood, and floor maps
should follow their own scales and rotations. Check the default gallery and
shadow-test scene for regressions, then exercise scene reload and shader reload.

## Local verification of patch 2

The production object and shadow GLSL were compiled and linked using Mesa's
llvmpipe software OpenGL context. Pixel checks verified transformed sampling,
visible/shadow alpha-mask agreement, and normal orientation for identity,
rotation, mirroring, and singular mappings. A diagnostic shader variant exposed
the production normal computation for those direction assertions. These checks
are not a substitute for Windows/NVIDIA runtime validation or performance tests.
