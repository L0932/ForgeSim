# Height displacement prototype (#132)

This implements CPU height displacement and the initial Sandbox material controls
for [issue #132](https://github.com/L0932/ForgeSim/issues/132). Displacement is off
by default and source models are not modified. This is bounded CPU subdivision,
not adaptive GPU tessellation or Blender shader-graph import.

## Use the Sandbox controls

1. Import the **original** GLB, with its unused height PNG embedded, or keep a
   separate height PNG available. Do not use the earlier `_displaced.glb`; that
   geometry is already baked. Prototype bake metadata is detected and prevents
   another displacement pass.
2. Select the object (or an imported node), expand **Materials → Material 1 →
   Height / Displacement**, then choose the embedded height PNG. For the concrete
   reference, choose `concrete_slab_wall_02_disp` and R/grayscale.
3. Enable displacement. Start with **Scale 0.050**, **Midlevel 0.225**, and
   **Subdivision levels 1**. Click **Apply height settings**; try level 2 for more
   geometry detail. Invalid images/channels and oversized geometry are rejected
   before the object settings or visible mesh are replaced.
4. Orbit the model to inspect silhouette, surface grooves and boundaries. Repeat,
   Mirror and Clamp controls affect height sampling only. Stitch smooth seams
   keeps geometry joined; it cannot erase a discontinuity painted into the height,
   normal or color images. The supplied sphere has no open geometric seam edges
   after exact-position welding, but its texture boundary still produces a ridge.
5. Save the scene. Height assignment and controls are saved per object/material;
   loading regenerates geometry from the original model and height image. Reset
   removes the override. Ctrl+Z/Ctrl+Y undo and redo applied settings. Duplicating
   an object copies its settings; editing the duplicate leaves the original alone.

**Browse height PNG...** uses a separate Windows file-dialog history. Images
already inside the asset root are referenced there. External images are copied to
`textures/heights` without overwriting an existing file. The scene stores a portable
relative reference. Keep the original model and referenced PNGs with the project;
missing external height files participate in asset recovery when loading scenes.
Embedding through Blender's **Unused images** exports image bytes, not the
material binding or scale/midlevel; those are assigned in ForgeSim.

Overrides affect all primitives using the selected authored material within the
selected object. Other instances share the source textures/materials while using
independent generated geometry. Render and shadow submissions, node queries,
bounds, focusing and picking use that geometry. Material/image indices assume the
same source export; if you replace the GLB, recheck assignments. UV0 is required;
height UV transforms are independent of the rendering maps; vector displacement is not implemented.

Scene files now use **version 6**; versions 1–5 load without height overrides when
the field is absent. Older ForgeSim versions will reject version 6 rather than
silently lose these settings. Save to a new scene filename for comparison if needed.

The editor retains up to 16 distinct generated configurations for undo/redo.
Each object is limited to 1,048,576 displaced vertices and 524,288 displaced
triangles across its selected materials. At the configuration limit, save/reopen
the scene to release old preview geometry. Excessive detail reports an error;
it is never silently reduced. File-dialog/native Windows rendering verification
still needs a Windows build; automated coverage runs headlessly.

## Match height mapping to tiled materials

Height sampling has independent **Height UV scale**, **Height UV offset**, and
**Height UV rotation (deg)** controls. These apply scale → rotate → translate,
followed by the selected height wrap mode. They do not change the mapping used
by color, normal, or roughness textures.

When the height image describes the same surface as the base-color image, click
**Use base-color UV mapping**, then **Apply height settings**. This copies the
base-color texture's UV0 transform into the height settings.

For example, a material whose color texture repeats twice along each axis may
need height UV scale `(2, 2)`. Match any offset and rotation as well. These values
depend on the asset; do not assume every tiled material uses the same mapping.

Choose the channel containing height data, usually **R / grayscale**. An alpha
channel containing only white produces a uniform offset rather than surface
detail. Height images are sampled as linear data.

Tune the remaining controls for the model:

- **Scale** controls the depth of the relief in mesh-local units. Start small
  relative to the model's dimensions and inspect the silhouette.
- **Midlevel** sets the height value that produces no displacement. Values above
  it move outward; values below it move inward when Scale is positive.
- **Subdivision levels** add geometry for sampling the height image. Each level
  multiplies the triangle count by four. Start low and increase as needed.
- **Wrap modes** determine sampling outside the image boundaries. Use Repeat for
  a repeating height texture.
- **Stitch smooth seams** keeps compatible coincident boundaries joined. It does
  not repair mismatched textures or distorted UV mapping.

Height UV settings save with each object/material override and participate in
undo/redo. Existing overrides without UV fields load with identity mapping.
After replacing a height image's contents on disk, reopen the scene to regenerate
geometry and clear retained preview configurations.

Third-party models and height images used during development are not bundled
with ForgeSim. The synthetic comparison described below requires no external
textures.

## Bake displacement into geometry in Blender

Use this workflow when the exported GLB should already contain displaced
geometry. ForgeSim then loads the resulting mesh without needing to interpret
Blender's displacement shader graph.

1. Save a working copy and select the mesh you intend to export.
2. Disconnect the shader Displacement output from the Material Output's
   Displacement input. Keep the base-color, roughness, normal-map, and surface
   connections.
3. Add a Subdivision Surface modifier **before** the Displace modifier. Use
   **Simple** to add geometry without smoothing the base shape. Choose the
   subdivision level according to the source mesh's density and required detail.
4. Assign a texture datablock to the Displace modifier. In Texture Properties,
   set its type to **Image or Movie**, load the height image, and use
   **Non-Color** data.
5. In the Displace modifier, set **Coordinates** to UV, choose the mesh's UV map,
   and set **Direction** to Normal.
6. Match the height texture's tiling to the material using the modifier texture's
   Mapping controls. Shader Mapping nodes do not configure this texture.
   Check that displaced features align with the color and normal maps.
7. Adjust **Strength** and **Midlevel** for the model. Strength controls relief
   depth; Midlevel sets the unchanged height value. Values that suit one model
   may be excessive or barely visible on another.
8. Export the selected mesh as **glTF Binary (.glb)** with **Apply Modifiers**,
   **UVs**, and **Normals** enabled. The exporter writes the evaluated geometry;
   the modifiers can remain editable in the working `.blend` file.
9. Import the GLB into ForgeSim and leave **ForgeSim displacement disabled**
   to avoid displacing the mesh twice.

Blender exports do not contain the ForgeSim prototype's bake metadata.
ForgeSim therefore cannot automatically identify these meshes as already
displaced.

If moving a `.blend` file breaks its image references, use **Find Missing Files**,
then **Make Paths Relative** and save. **Pack Resources** can embed the resolved
images in the `.blend` file. Check missing environment HDR images separately from
the material's height image.

## Known limitation: sphere poles

UV spheres can show radial texture stretching where their mapping converges
at the poles. Displacement can make this distortion more noticeable, and baking
preserves it. Additional subdivision and seam stitching do not correct the UVs.

Use a flat, subdivided plane to inspect displacement without spherical mapping
distortion. Improving a sphere's pole mapping is a separate asset-authoring task.
Any UV changes must keep height, color, roughness, and normal maps aligned.

## Validation checklist

- Save and reopen a scene; confirm height settings, UV mapping, and regenerated
  geometry are preserved.
- Duplicate a displaced object and confirm edits to the duplicate leave the
  original unchanged.
- Verify undo/redo restores both the applied settings and visible geometry.
- Inspect displaced silhouettes, shadows, focusing, and picking.
- Import a Blender-baked GLB with ForgeSim displacement disabled.
- Run the Windows build and CTest suite, and record the results in the PR.
  Automated headless checks do not verify native file dialogs or interactive
  rendering.

## Build and run

Enable the opt-in target in your existing build tree:

```bash
cmake -S . -B out/build/x64-Debug -DFORGESIM_BUILD_DISPLACEMENT_PROTOTYPE=ON
cmake --build out/build/x64-Debug --config Debug --target ForgeSimDisplacementPrototype ForgeSimMeshDisplacementTests ForgeSimGlbDisplacementPrototypeTests
ctest --test-dir out/build/x64-Debug -C Debug -R "MeshDisplacement|GlbDisplacementPrototype" --output-on-failure
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

## Bake the concrete sphere GLB

The GLB mode retains the input's material, textures, samplers, nodes and scenes.
It changes the selected primitive's geometry and adds prototype bake provenance
under root `extras.forgeSimHeightBake`. Original binary data remains in the output,
so this is not a size-optimized export. Use the original input for each bake;
already-baked inputs and existing output filenames are rejected.

```bash
./out/build/x64-Debug/tools/displacement/ForgeSimDisplacementPrototype.exe --input "C:/path/concrete_slab_wall_02_1k(1).glb" --output concrete_slab_wall_02_displaced.glb --embedded-height concrete_slab_wall_02_disp --levels 2 --midlevel 0.225 --scale 0.050
```

Import the resulting GLB in ForgeSim using the existing model workflow. It already
contains displaced geometry; no height setting in Sandbox is required.

`--mesh` and `--primitive` select zero-based indices (both default to zero).
Without `--embedded-height`, the tool matches the selected material name followed
by `_disp`, `_displacement` or `_height`, optionally with an underscore suffix.
Missing or multiple matches require an explicit choice. Alternatively, supply
`--height path.png`; external and embedded selectors are mutually exclusive.
This naming convention is a prototype rule, not a standard glTF height binding.

GLB mode defaults to two subdivision levels and Repeat sampling; plane mode
defaults to six levels and Clamp to Edge. Only static triangle primitives with
POSITION, NORMAL, TEXCOORD_0 and optional TANGENT attributes are supported.
UV0 is required. Extra attributes, primitive extensions and morph targets are
rejected rather than silently discarded. Skinning is not supported in the prototype GLB tool. Other primitives remain unchanged; all instances of the selected
mesh use its baked geometry.

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
  Plane mode uses Clamp to Edge; GLB mode and library defaults use Repeat.
- Subdivision splits each indexed triangle into four and shares midpoint
  vertices for shared indexed edges. Position, UV and vertex color interpolate;
  source normals interpolate before height evaluation. Normals and glTF tangent
  frames are rebuilt from the final displaced triangles.
- GLB mode stitches exact coincident vertices with matching normalized source
  normals by averaging their displaced positions, keeping their UVs and indices
  separate, then sharing area-weighted rebuilt normals. This prevents gaps where
  UVs disagree at smooth seams and sphere poles. Averaging changes the sampled
  boundary height intentionally; it is not a claim of Blender parity. Use
  `--no-stitch-seams` to disable it. The library setting defaults to off.
  Hard-normal boundaries and separate primitives remain separate; there is no
  tolerance-based weld or self-intersection repair.
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

The uploaded concrete sphere was baked using its embedded **16-bit grayscale
PNG**, midlevel `.225`, scale `.050` and two subdivision levels. It grows from
9,004 vertices / 16,128 triangles to 132,643 vertices / 258,048 triangles.
Material, image, texture, sampler, node and scene JSON remained unchanged, as did
the original binary prefix. Coincident source vertices had zero position and
normal separation after stitching (unstitched pole gaps reached about `.0367`
mesh units). This is a structural check, not a completed visual review in ForgeSim.
The original Blender file has not been supplied, so exact Blender parity remains
unverified.

Additional automated GLB checks exercise native 16-bit height decoding, preserved
material/image metadata, actual ForgeSim importer reload, ambiguous/missing image
selection, explicit assignment, conflicting selectors and double-bake protection.
Mesh tests also cover smooth-seam averaging, retained UV splits and hard normals.

Next stages include automatic height suggestions, more general
seam/material-boundary handling, and visual review of the concrete reference.
The initial Inspector controls, undo/redo and per-object scene persistence are
implemented. Automatic Blender displacement metadata import remains future work.

Normal/bump maps change shading without changing silhouettes. Parallax occlusion
changes apparent surface depth in the shader. This prototype changes mesh
positions, so it can change silhouettes and geometry used by shadow passes.

Reference material: [Poly Haven concrete_slab_wall_02](https://polyhaven.com/a/concrete_slab_wall_02).
The reference texture is not included in this patch. The generated synthetic
comparison requires no third-party textures.
