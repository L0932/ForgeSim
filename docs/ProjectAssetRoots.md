# Project asset roots

Scene opening through **File > Open Scene**, the local-scene Samples browser,
and `--scene` uses the same root discovery policy. The Console command `project.root "directory"` sets an explicit
override for subsequent ordinary opens; `project.root.auto` restores discovery.
Querying `project.root` does not disable discovery. A CLI `--project-root` remains
an explicit override for editor opens until cleared.

## Manifest format

Place `ForgeSim.project.json` in the project or sample-pack folder:

```json
{
  "version": 1,
  "assetRoot": "."
}
```

`assetRoot` is required. Use `.` for the manifest folder, or a portable relative
subfolder such as `content`. Paths use forward slashes; absolute paths, drive
prefixes, empty components, and `.` or `..` components within a subfolder path
are rejected. The asset-root directory must already exist. Unknown fields are
ignored; unsupported manifest versions are rejected.

Scene model and project-environment references remain relative to this asset
root. The manifest does not change scene documents or copy assets. Keep the
manifest, scenes, and required assets together when moving a project or pack.

## Resolver policy

1. An explicit root override wins and must name an existing directory.
2. Otherwise search the scene's containing directory and each ancestor for the
   nearest `ForgeSim.project.json`. Resolve its asset root against that manifest.
3. If no manifest exists, retain the caller's legacy root (normally the executable
   directory).

The scene must be an existing regular file. Callers should pass an absolute scene
path: a relative scene path has the usual filesystem meaning at the API boundary.
Once the scene is resolved, discovery does not use the working directory. An
invalid or unreadable nearest manifest fails visibly instead of trying a parent
manifest or legacy root. Results identify the chosen policy and manifest path.

## Opening and recovering scenes

Discovery selects an asset root; scene loading then validates model and
project-environment references. Bundled sample menus always use their bundled
root. One manifest beside the executable also covers bundled scenes opened
through the ordinary scene dialog.

The Samples browser requires only an absolute scene path; use **Browse scene...**
to select it. **Allow texture fallbacks** is checked by default for local sample
opens and applies only to that load. Ordinary scene opens retain the global
texture policy.

When editor loading finds missing model or project-environment files, a recovery
window lists their stored references, attempted root, and resolved paths. Use:

- **Locate model...** to select a GLB or glTF file and infer its root from the
  stored reference.
- **Choose asset folder...** to select the folder those references are relative to.
- **Retry** after restoring missing files.
- **Cancel** to keep the current scene.

Every reference is checked again before a replacement is adopted. Cancellation
and failed loading preserve the active scene and its edits. The chosen root
applies only to that prepared scene; no document or global override is rewritten.

This initial recovery handles missing scene-level files during editor opens.
Invalid manifests, texture fallback prompts, remembered per-scene roots, and
startup recovery UI remain separate work. CLI errors still report details.

Locating a model does not rewrite scene references. For a reference such as
`Cathedral_01/cathedral.glb`, selecting `C:/Assets/Cathedral_01/cathedral.glb`
infers `C:/Assets`. A file in `Cathedral_01-test` cannot satisfy that unchanged
reference: restore the original folder name or update the authored reference.

## Creating a project or pack

Create the manifest manually for now; saving a scene does not create one. For
`assetRoot: "."`, a pack can contain `ForgeSim.project.json`,
`scenes/Example.scene.json`, and `models/Example.glb`. The scene stores
`models/Example.glb`, and discovery finds the manifest above `scenes`.

Save scenes inside the project tree. A scene saved in the default user-data
folder cannot discover a manifest in an unrelated asset folder. Such legacy
scenes can still use explicit root overrides or editor recovery.

## Verification before merging

Build and run the headless tests in Windows Debug and Release. Check these
editor workflows as well:

- Open both bundled samples and a scene from an extracted project pack.
- Move the complete pack and open it again without a root override.
- Launch from a different working directory with an absolute `--scene` path.
- Verify explicit CLI/Console overrides, then clear them with `project.root.auto`.
- Retry missing assets using both pickers; try an incorrect folder and cancel.
- Confirm cancellation preserves unsaved edits, selection, and undo history.
- Check the local-scene texture-fallback checkbox with strict and fallback loads.
- Move/resize the Samples window and reset the layout, including while it is closed.
