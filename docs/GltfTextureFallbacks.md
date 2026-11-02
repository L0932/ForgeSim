# Optional glTF texture fallbacks

Sandbox imports retain the existing strict behavior unless explicitly launched
with `--allow-texture-fallbacks`. For example, from the repository root in Git Bash:

```bash
"./out/build/x64-Debug/apps/Sandbox/ForgeSimSandbox.exe" --allow-texture-fallbacks --project-root "C:/ForgeSimAssets" "C:/ForgeSimAssets/Cathedral_01/paul_statue.glb"
```

The flag also works with `--scene FILE` and applies to model imports made when
opening or creating scenes during that session. It is an import policy, not an
authored scene setting, and is not saved in scene documents. Relaunch with the
flag when reopening a scene that requires fallback imports.

## What changes

For each primitive, the fallback skips individual base-color,
metallic-roughness, normal, occlusion, or emissive texture usages when:

- The material selects a negative texture-coordinate set.
- The material selects a set other than `TEXCOORD_0`, the only set currently
  supported by ForgeSim's material rendering path.
- The primitive has no `TEXCOORD_0` attribute.

Material color, metallic, roughness, emissive, alpha-mode/cutoff, and double-sided
properties remain authored. Valid texture usages remain active. Primitives that
share a source material receive distinct variants only when their skipped usages
differ; a primitive without UVs does not remove another primitive's textures.

Every skipped primitive texture usage reports a warning containing the asset
path, mesh and primitive indices, material index/name, texture usage, selected UV
set, and reason. The startup log also reports that fallback mode is enabled.

No UVs are generated and invalid set indices are never silently changed to zero.
This is a compatibility aid; skipped textures can visibly change color, cutout
silhouettes, surface detail, and shadows. Correcting the source asset remains the
way to restore its authored appearance.

Malformed geometry, malformed UV accessors, failed image decoding, missing files,
and unrelated validation failures still fail the import. Images are still decoded
through the existing resource-loading path, including images whose texture usages
were skipped. This option does not reduce large-file memory or loading costs.

## Asset API

`LoadGltfScene(path)` retains the existing strict behavior. To opt in, pass
`GltfImportOptions` with `GltfTexturePolicy::SkipUnusableTextures`. An optional
`onWarning` callback receives synchronous warning messages; copy a message if it
must outlive the callback. The loader itself does not depend on Sandbox logging.
`LoadGltfMesh` remains a geometry-only convenience loader.

## Validation

Build and run the focused loader tests, then build Sandbox:

```bash
cmake --build out/build/x64-Debug --target ForgeSimGltfLoaderTests ForgeSimSandbox --config Debug
ctest --test-dir out/build/x64-Debug -C Debug -R '^ForgeSim\.Assets\.GltfLoader$' --output-on-failure --no-tests=error
```

Repeat with `out/build/x64-Release` and `Release` before completing the issue.

Manual checks:

1. Run Sandbox with `--help`; confirm the fallback flag is documented.
2. Load an affected model without the flag; confirm its existing error remains.
3. Load it with the flag; confirm warnings identify affected primitives/materials
   and unaffected surfaces retain their textures.
4. Save the scene, start a new scene, and reopen the saved scene within the same
   session; confirm fallback imports still work.
5. Restart with the saved scene and the flag; confirm it loads. Restart without
   the flag; confirm the existing strict behavior returns.
6. Load the bundled material gallery without the flag and confirm its appearance
   remains unchanged. Check that missing/corrupt model loads leave an existing
   scene intact through the existing scene-resource preparation path.
