# Directional Shadow Validation

This scene provides a solid receiver and two independently editable
casters. It uses original ForgeSim geometry and no external textures.

## Launch From a Source Checkout

Build the Sandbox, then run this command from the repository root:

```text
"./out/build/x64-Debug/apps/Sandbox/ForgeSimSandbox.exe" --project-root "./apps/Sandbox" --scene "./apps/Sandbox/assets/scenes/DirectionalShadowTest.forgesim.json"
```

The project root resolves the scene's `assets/models/ShadowTestCube.gltf`
reference. It does not change the shader directory printed at startup.
Replace x64-Debug with your actual build-directory name when necessary.

For a packaged copy, run from the directory containing the executable:

```text
./ForgeSimSandbox.exe --project-root . --scene assets/scenes/DirectionalShadowTest.forgesim.json
```

## Scene Layout

- Floor: a 12 by 12 slab with its top at Y = 0.
- Resting Cube: a 1.5-unit cube with its bottom at Y = 0.
- Floating Cube: a 1.5-unit cube with its bottom at Y = 1.25.
- One enabled directional light; no point or spot lights.

All objects share one centered cube mesh. Their separate scene-object
transforms make them independently editable within the current shared-
model scene format. Select a cube by name in the hierarchy when the floor
is easier to hit with mouse picking.

## Viewing Conditions

Navigate above the floor and look down toward the cubes. The initial
camera frames the full scene and may need repositioning for a close view.
Set Environment intensity to about 0.03 and Exposure to about 1.0.
Disable the reference grid, bounds, light guides, and selection highlight
when comparing images. These debug settings are not stored in the scene.

## Checks

1. Toggle directional shadows. Cast shadows on the solid floor should
   appear and disappear; the receiving floor itself remains visible.
2. Change the directional light's direction. Both shadows should move.
   Moving only its editor position should not change them.
3. Select Floating Cube and increase its Y position. Its floor shadow
   should shift farther away for the angled light. Moving or rotating
   that cube must not move the floor or Resting Cube.
4. Inspect Resting Cube at its base. Its shadow should meet the cube
   without severe speckling or a large detached gap.
5. Toggle PCF filtering and inspect an edge closely. Filtering smooths
   the pixel boundary; it does not implement distance-dependent softness.
6. Increase minimum or slope bias to see the tradeoff between reduced
   acne and detached shadows, then restore reasonable values.
7. Resize and minimize/restore the window, and exercise selection,
   gizmos, undo, and redo. The visible pass must remain correct.

Changing scene bounds currently refits the projection, which can shift
shadow texels slightly while moving objects. Large quality changes or
missing shadows still require investigation.

Use Save As to keep experiments separately from this baseline scene.
Scene documents reference their model asset; they do not embed it.

## Implementation

- One 2048 × 2048 depth map for the main directional light.
- An orthographic shadow projection fitted to current scene bounds.
- Shadows affect directional direct lighting; environment lighting,
  point lights, spot lights, and emissive contributions remain separate.
- Optional 3 × 3 PCF with bilinear interpolation of depth comparisons.
- Receiver-plane depth correction reduces filtering-induced acne
  at grazing light angles.
- Runtime controls provide shadow enablement, filtering, minimum
  bias, slope bias, and reset to defaults.

## Editor behavior

Light-marker size and picking radius are established when a scene
loads and remain stable during object editing. Shadow projection
bounds continue updating with scene geometry.

## Known limitations

- A single shadow map shares its resolution across the entire scene.
  Large scenes can lose fine shadow detail.
- Filtering softens sampled edges; it does not simulate physically
  accurate soft shadows.
- Bias may require adjustment for different scenes.
- Receiver-plane correction approximates the receiving surface
  locally and can be less reliable near silhouettes or degenerate
  projections.
- Point-light and spot-light shadows are not implemented.
- Shadow debug settings are not persisted between sessions.