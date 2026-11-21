# glTF normal-map handedness

For glTF tangent frames, the shader reconstructs `B = cross(N, T) * tangent.w`.
Authored `TANGENT` attributes already encode this convention and are preserved.

When generating missing tangents from glTF UV coordinates, the raw UV-derivative
bitangent follows increasing V. glTF texture coordinates have their origin at
the top left, whereas normal-map +Y points up. The generated sign must therefore
oppose the raw derivative bitangent. The fragment shader's derivative frame for
transformed normal-map UVs follows the same rule. Texture coordinates, texture
pixels, and authored tangent signs are not flipped.

This fixes normal-map green perturbations being reversed for assets without
stored tangents. It does not replace the custom tangent averaging algorithm with
MikkTSpace, change normal strength, or alter environment filtering.

## Brass-pot investigation for issue #133

The supplied `brass_pot_02_1k.glb` contains one primitive with 2,086 vertices,
3,344 triangles, normals, UVs, and no tangent attribute. Its material is
double-sided, with normal strength and metallic/roughness factors defaulting to 1.
Its normal image is a 1024 x 1024, 16-bit RGB PNG. ForgeSim currently decodes that
image to RGBA8; that precision limitation is separate from this sign correction.

A headless comparison used the same GLB and Clarens Midday 4K HDRI, Blender 4.3.2
EEVEE, ForgeSim's actual GLSL and environment processor under software OpenGL 4.6,
Standard/sRGB output, neutral exposure, environment strength 1, and no direct
lights. The environment's vertical orientation was aligned. The camera was:

- Model-relative position: `(0.123579, 0.431613, -0.314912)` in ForgeSim Y-up.
- Model-relative target: `(-0.120421, -0.313387, 0.306088)`.
- Vertical field of view: 45 degrees; test framebuffer: 864 x 496.

Emissive diagnostic passes isolated UVs, texture samples, geometric normals, and
mapped normals from lighting. UVs, sampled normal texture values, and geometric
normals agreed closely. In the interior region x=330..499, y=180..259
(bottom-left pixel origin), changing only bitangent handedness reduced mapped
normal error against EEVEE from a median 3.20 degrees to 0.22 degrees, and the
90th percentile from 6.87 to 0.40 degrees. The original pixel comparison used
ForgeSim's generated tangent directions; the final render also exercised the
corrected C++ generation routine directly.

Increasing the specular map from 128 to 512 pixels did not recover the missing
patterns with the old sign. Reducing Blender's normal texture to 8-bit retained
the larger patterns. The final corrected render recovered the darker interior
variation without increasing normal strength or changing material factors.
These observations identify the handedness defect; they do not establish exact
rendering parity across Blender versions, tangent generators, or IBL models.

## Verification

- `ForgeSim.Assets.MeshProcessing`: generated-frame signs, mirrored UVs,
  normal-map green direction, authored-attribute preservation, degeneracy.
- `ForgeSim.Assets.GltfLoader` and `ForgeSim.Assets.TextureTransform`.
- Headless GPU checks of the actual `BuildNormalBasis` shader function: authored
  frame, derivative identity, nonuniform UV scale, 90-degree rotation, mirror U,
  mirror V, singular mode, and collapsed derivatives.
- Manual: reload the same GLB, keep selection tint disabled, and compare the
  pot's bottom against Blender with normal strength 1 and the matched camera.

The Windows application build and interactive validation remain to be run on
the target machine.

## References

- [glTF normal texture conventions](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#material-normaltexture)
- [Khronos discussion of generated tangent signs and UV origin](https://github.com/KhronosGroupArchives/glTF-Sample-Models/issues/174)
