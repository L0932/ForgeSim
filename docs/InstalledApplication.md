# Running the Installed ForgeSim Sandbox

## Launch

Extract or copy the complete application folder to a suitable location,
then launch ForgeSimSandbox.exe.

Keep the assets directory and ForgeSim.project.json beside the executable. Moving only the
executable will leave required resources behind.

The application can be launched from another working directory.

## Graphics Requirements

The current renderer requires an OpenGL 4.6-capable graphics driver.

Install an appropriate graphics driver from your hardware vendor if
the application cannot create its graphics context.

## Bundled Content

The installation includes the PBR material gallery, a basic cube,
an HDR environment, and the shaders required by the renderer.

Additional models and purchased showcase assets are not included.

Asset sources and attribution are recorded in:

- assets/models/README.md
- assets/environments/README.md

Dependency notices are under notices/.

## Sample Scenes

Choose **File > Open Sample > Starter: Directional Shadows** or **Material Gallery**.
Both use bundled assets and open as editable copies. Save asks for a user file;
the installation's scene documents are not overwritten. Unsaved edits are
confirmed before switching scenes.

The menu chooses the bundled asset root automatically, including after moving
the whole application folder. It leaves your Console `project.root` setting intact.
Saved sample copies still reference bundled assets: before reopening a copy with
Open Scene, set `project.root "C:/Path/To/ForgeSim"` if you have selected an external
root. Saving a scene does not copy its assets.

For an external scene, choose **File > Open Sample > Browse Samples...** and select
its ForgeSim scene JSON. The nearest project manifest establishes its asset root;
explicit root overrides take precedence. This opens a copy using that root
without changing the Console setting. **Allow texture fallbacks** is checked by
default for local sample opens and applies only to that load. External assets
must already be available on your computer.

See [SampleScenes.md](SampleScenes.md) for distribution and asset provenance.

## User Data

On Windows, ForgeSim stores application-managed data under:

%LOCALAPPDATA%/ForgeSim/

- Settings/imgui.ini stores the editor layout.
- Scenes/ is the default scene-file directory.

The native file dialog may restore its last-used folder instead of the
default directory.

Explicitly selected scene destinations remain supported. Scene files
reference assets; saving a scene does not copy those assets.

## Scene Asset References

When opening a scene, ForgeSim searches upward from its folder for the nearest
`ForgeSim.project.json`. Model and project-environment references resolve against
the manifest's asset root. Keep scenes beneath their project folder and move the
complete project to preserve these references. Without a manifest, the application
folder remains the legacy fallback.

An explicit `--project-root` or Console `project.root "directory"` overrides
manifest discovery. Use `project.root.auto` to clear the override for future
editor opens. Bundled sample menu actions always use their bundled assets.

Load saved scene documents with `--scene "C:/MyProject/scenes/Example.scene.json"`.
Without a model path or `--scene`, startup opens the bundled
`assets/scenes/MaterialGallery.scene.json` beside the executable, including its
saved camera position, movement speed, lighting and rendering settings. This
bundled startup uses its own asset root even when `--project-root` is supplied.
To update it in a source build, replace
`apps/Sandbox/assets/scenes/MaterialGallery.scene.json` and rebuild; the build
copies it beside the executable. Keep its asset references relative to the
Sandbox project root. The installed version uses the scene shipped in its package.

A positional `.glb` or `.gltf` path imports a model instead. Saving a model as a
scene does not create a project manifest or copy its assets.

For missing model or project-environment files, editor opens offer **Locate
model...**, **Choose asset folder...**, **Retry**, and **Cancel**. Located assets
must match the stored relative references. Failed or cancelled opens preserve the
active scene. Invalid manifests and CLI startup failures report errors directly.

See [ProjectAssetRoots.md](ProjectAssetRoots.md) for the manifest format and
recovery behavior.

Moving the application does not rewrite external scene documents or
copy external project assets.

## Development Shader Override

By default, shaders load from assets/shaders beside the executable.

FORGESIM_SHADER_DIRECTORY can override that directory for development.
Remove the override when checking an installed build.

## Troubleshooting

Startup diagnostics report the executable, asset, shader, and layout
settings locations.

When reporting a problem, include the application version, operating
system, graphics hardware, and relevant diagnostic output.
