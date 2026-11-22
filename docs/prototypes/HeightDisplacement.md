# Height displacement prototype (#132)

This is the first technical prototype for [issue #132](https://github.com/L0932/ForgeSim/issues/132).
It is **not** the complete material override or editor workflow. Default Sandbox
rendering is unchanged. No source assets or existing scene documents are modified.

## Build and run

Enable the opt-in target in your existing build tree:

```bash
cmake -S . -B out/build/x64-Debug -DFORGESIM_BUILD_DISPLACEMENT_PROTOTYPE=ON
cmake --build out/build/x64-Debug --config Debug --target ForgeSimDisplacementPrototype ForgeSimMeshDisplacementTests
ctest --test-dir out/build/x64-Debug -C Debug -R MeshDisplacement --output-on-failure
```

From Git Bash, generate a comparison GLB (choose a new output filename):

```bash
./out/build/x64-Debug/tools/displacement/ForgeSimDisplacementPrototype.exe --output comparison.glb --levels 6
```

The GLB contains two one-unit planes: a flat control on the left and baked CPU
displacement on the right. Without `--height`, the tool uses a synthetic linear
sine height field. Import the GLB through ForgeSim's existing model workflow,
select the displaced node, and inspect it from a low angle. A directional light
and a separate receiving surface can be added to review shadows.

For the concrete reference, supply the height image explicitly:

```bash
./out/build/x64-Debug/tools/displacement/ForgeSimDisplacementPrototype.exe --output concrete-comparison.glb --height "C:/path/concrete_slab_wall_02_disp.png" --levels 8 --midlevel 0.225 --scale 0.050 --channel 0
```

The test asset uses a neutral material, not the original concrete color/normal
textures. A `--help` option lists the available prototype controls. `--invert`
reverses the height samples. Output filenames are never silently overwritten.

## Conventions

- Decode PNG height channels as **linear data**; do not apply an sRGB transfer.
  Native 8- and 16-bit PNG samples are normalized to float without reducing
  16-bit sources to 8-bit. Channels are zero-based R/G/B/A where available;
  grayscale has only channel zero. EXR and floating-point height formats are
  not implemented in this stage.
- Positive displacement follows interpolated source normals:
  `offset = (height - midlevel) * scale`. Inversion uses `1 - height` before
  subtracting midlevel. At `.225` and `.050`, black offsets by `-.01125` units
  and white by `+.03875` units.
- Units are mesh-local engine units, before glTF node and object transforms.
  Uniform object scale 2 doubles world displacement; nonuniform scale also
  scales the displaced geometry. This is the initial Object Space convention,
  not a world-space distance guarantee.
- glTF UV0 is used directly, with row zero at the top of the height image.
  Bilinear sampling supports Repeat, Mirrored Repeat and Clamp to Edge.
  The comparison tool uses Clamp to Edge; library defaults use Repeat.
- Subdivision splits each indexed triangle into four and shares midpoint
  vertices for shared indexed edges. Position, UV and vertex color interpolate;
  source normals interpolate before height evaluation. Normals and glTF tangent
  frames are rebuilt from the final displaced triangles.
- UV splits, separate primitives and hard-normal boundaries are not welded or
  stitched. Inconsistent heights/normals across them can produce gaps. Start
  with a continuous UV-mapped plane; closed/seamed production meshes need
  additional policy before editor integration. Skinning, morph deformation,
  self-intersection handling and UV transforms are not addressed here.
- Disabled displacement or zero scale preserves source topology and attributes
  without requiring a height image.

## Limits and measured cost

Subdivision is limited to eight levels. Default budgets are 1,048,576 vertices
and 524,288 triangles. A conservative vertex upper bound and exact triangle
count reject oversized requests before subdivision allocates output geometry.
Height images are limited to 16M pixels and 16,384 pixels on either axis;
encoded input files are limited to 256 MiB. These limits do not define total
process memory: image decoding and subdivision edge caches add temporary cost.

Representative release-build CPU generation results on the development Linux
container, median of three runs per level with the synthetic image:

| Levels | Vertices | Triangles | Mesh storage | Generation |
| --- | ---: | ---: | ---: | ---: |
| 4 | 289 | 512 | 23,484 bytes | 0.09 ms |
| 6 | 4,225 | 8,192 | 351,804 bytes | 1.12 ms |
| 8 | 66,049 | 131,072 | 5,535,804 bytes | 32.47 ms |

Mesh storage counts `MeshVertex` and index buffers only, not texture data or peak
allocation. Generation timing excludes decoding, file output and GPU upload;
it is not a frame-time measurement or a Windows performance guarantee. Runtime
cost increases in both visible and shadow passes as triangle counts grow.

## Validation and remaining work

Automated checks cover linear channel sampling, 16-bit precision, wrap behavior,
scale/midlevel/inversion, normal and tangent generation, shared-edge subdivision,
no-op preservation, invalid data, and pre-allocation geometry budgets. The
comparison GLB was loaded successfully by ForgeSim's actual glTF importer;
its displaced vertices extend the mesh's Y bounds above and below zero.

Because the prototype bakes positions into a normal GLB, existing rendering,
shadow upload, bounds and picking receive actual changed geometry rather than
a fragment-only effect. Visual shadow matching and interactive focus/picking
still require review in ForgeSim before this issue's acceptance criteria are met.

Next stages include per-instance/per-material overrides, undo/redo, persistence,
explicit metadata, matching embedded/local height images, ambiguity handling,
Inspector assignment controls, transformed UVs, seam/material-boundary policy,
and a reproducible concrete reference scene. The original reference GLB/.blend
and height image have not yet been provided for this implementation, so their
exported texture contents and Blender parity have **not** been verified.

Normal/bump maps change shading without changing silhouettes. Parallax occlusion
changes apparent surface depth in the shader. This prototype changes mesh
positions, so it can change silhouettes and geometry used by shadow passes.

Reference material: [Poly Haven concrete_slab_wall_02](https://polyhaven.com/a/concrete_slab_wall_02).
The reference texture is not included in this patch. The generated synthetic
comparison requires no third-party textures.
