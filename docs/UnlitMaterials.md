# Unlit glTF materials

ForgeSim supports `KHR_materials_unlit` in both optional and required glTF
extension declarations. The extension applies to each material that contains
`"extensions": { "KHR_materials_unlit": {} }`; declaring the extension at the
asset level does not make every material unlit.

Unlit surface color is the product of base-color factor, base-color texture and
supported vertex color. Material UV bindings and base-color texture transforms
continue to apply. Unlit surfaces ignore direct lighting, received shadows,
environment lighting, metallic/roughness, normal maps, occlusion and emissive
inputs. Opaque, masked and blended alpha modes and double-sided rasterization
keep their existing behavior. Editor selection highlighting remains available.

Unlit geometry may still cast directional shadows on lit surfaces. Shadow casting
is controlled independently by the existing per-node `castsDirectionalShadows`
setting. Unlit output remains in the HDR pipeline: exposure and tone mapping can
change its displayed appearance. Keep exposure fixed when comparing lighting.

## Regression fixture

`tests/assets/UnlitMaterials.gltf` is a self-contained synthetic model containing
four named panels, from left to right: lit opaque, unlit opaque, unlit masked and
unlit blended. It includes original quad geometry, vertex colors, primary UVs and
an embedded 2-by-2 nearest-filtered RGBA texture with zero, partial and full alpha.
It contains no Kenney model data or other external assets.

`GltfUnlitTest.cpp` runs through the existing glTF loader test target. It checks:

- Required-extension import through scene and mesh loading.
- Both glTF and a generated GLB container.
- Optional-extension import.
- Per-material flags in an asset mixing lit and unlit materials.
- Authored base-color factors, texture bindings, vertex colors, UVs, alpha modes,
  cutoff, sidedness and decoded texture alpha.
- Rejection of unknown required extensions.

These are CPU loader tests; they do not verify GPU blending or fragment output.

## Sandbox rendering checks

Import the fixture using the normal external-model workflow. Use a contrasting
background and deselect all panels so highlighting does not affect comparisons.
Keep exposure fixed throughout these checks.

1. Set all direct-light intensities and environment-light intensity to zero.
   The three unlit panels should retain their colors while the lit panel darkens.
2. Restore lighting and change its direction and intensity. The lit panel should
   respond; the unlit panels should retain their appearance.
3. The unlit opaque panel must cover its entire quad even where texture alpha is
   zero. The masked panel must have hard cutout edges and no intermediate alpha.
   The blended panel must show the background through partial-alpha texels.
   Put a lit opaque object behind the blended panel to check depth occlusion and
   blending; no change to the existing transparent-sorting limitations is intended.
4. Inspect all panels from the back. The lit panel is single-sided; the three
   unlit panels are double-sided and should remain visible.
5. Place a shadow caster in front of the panels. Received shadows may darken the
   lit panel, but must not darken the unlit panels. Unlit geometry may cast shadows
   onto a separate lit floor.
6. Select each unlit panel to verify selection tint, then deselect it. Reload the
   ordinary Material Gallery and verify that lit materials still respond normally.
7. Import representative Kenney Retro Fantasy models, including `barrels.glb` and
   `battlement.glb`, from a separately downloaded pack. Do not add the pack here.

For a configured Debug build, run:

```bash
cmake --build out/build/x64-Debug --config Debug
ctest --test-dir out/build/x64-Debug -C Debug --output-on-failure
```

Also build and test the separately configured Release build using its own build
directory and Release configuration before merging. The loader tests cannot
replace the Sandbox rendering checks above.

Specification: [Khronos KHR_materials_unlit](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_unlit/README.md).
