# ForgeSim Sample Scenes

## Open a bundled sample

Build the Sandbox or extract the complete official application package. In the
editor, choose **File > Open Sample**:

| Sample | Purpose | Required bundled assets |
| --- | --- | --- |
| Starter: Directional Shadows | Select and transform cubes, toggle visibility, and inspect directional shadows on a floor. | `ShadowTestCube.gltf` |
| Material Gallery | Compare metallic and nonmetallic materials across roughness values. | `PbrMaterialGallery.glb` |

The cards use screenshots of the bundled scenes and preserve their aspect ratio.
At narrower browser widths they stack vertically. A missing preview does not
prevent opening the scene.

Both use the bundled studio environment and require no separate downloads,
texture fallback settings, project-root command, or scene JSON edits. The starter
cube contains its own geometry and has no external textures. The application
frames each sample automatically.

Samples open as editable copies. **Save** asks for a destination in your scene
folder rather than writing into the installation. Opening a sample uses the
normal unsaved-changes confirmation. A failed open leaves the active scene in place.

Keep `assets` beside the executable. Launching from a different working directory
or relocating the whole application folder is supported. Opening a sample uses
the bundled root even if `project.root` currently points to another project;
it does not change that Console setting for subsequent ordinary scene opens.

Saving a scene writes references, not copies of its models. To reopen a saved
sample copy through **File > Open Scene**, use the application directory as the
project root if you have previously selected an external root:

```text
project.root "C:/Path/To/ForgeSim"
```

Then open your saved scene. The root is the folder containing `assets`, not the
scene-file directory. Saved copies still depend on the application assets.
Automatic root discovery and missing-asset recovery are separate follow-up work.

## External samples in the browser

Choose **File > Open Sample > Browse Samples...**. The Samples window shows
screenshot cards for the bundled examples. Expand **Open a local scene** to use
a scene and assets already on your computer. No additional third-party assets are downloaded or included.

1. Select an existing ForgeSim scene JSON with **Browse scene...**, or paste its
   path. A relative scene path is resolved against the root you enter below.
2. Enter the absolute **Project root** folder. For a scene containing
   `models/example.glb` and a model at
   `D:/SamplePack/models/example.glb`, enter `D:/SamplePack`.
3. Click **Open local scene**. Confirm any unsaved edits as usual.

The scene opens as an editable copy and uses the selected root. Save asks for a
user destination. The Console `project.root` setting is unchanged. The browser
keeps its input fields during the current application session; it does not yet
maintain a persistent external-sample catalog.

This opens scene documents, not raw `.gltf`, `.glb`, or Blender files. To prepare
an external sample, import its model through the existing launch workflow,
configure lighting and a saved viewpoint, then save a ForgeSim scene JSON. The
root must match the relative asset paths stored in that JSON. The browser does
not convert models, rewrite asset paths, or grant redistribution rights.

Local scenes require their own assets and any necessary acquisition or import
steps. Existing runtime texture-fallback and
cache settings continue to apply to sample loads.

## Source checkout

Use the bundled scenes with the executable and assets from the same build or
release package. The installed copy of this guide identifies its ForgeSim
version. The scene browser is development work after `0.1.0-preview.2`; that
release and earlier do not include it. No standalone external sample pack has
been released yet, so there is no separate pack minimum-version claim.

The scene documents live in `apps/Sandbox/assets/scenes`. Building the Sandbox
copies them and their model dependencies next to the executable. Official install
rules explicitly include both scenes. Use a build containing **File > Open Sample**;
older builds may open the documents manually with the correct project root but
will not provide the editable-copy workflow. Saving uses the current scene schema;
older application versions may not support the resulting file.

## Distribution strategy

Keep small, self-contained starter scenes and their required assets in the
repository and official application packages. A fresh installation must always
have an example available without a separate download.

Large showcases should be optional, versioned sample packs attached to a ForgeSim
release, with download links from the repository and site. This is the planned
channel; no additional downloadable pack is published by this change. Each pack
should include:

- Scene documents and all assets whose redistribution is permitted, with portable
  paths relative to one documented pack root.
- A README covering minimum ForgeSim version, unpacking, project-root selection,
  opening, expected features, and any required import options.
- Credits and license terms for every included asset, plus source links and any
  modifications. Do not assume a model's license covers every dependency.
- A file list, archive size, and checksum alongside the release download.

For assets that cannot be redistributed, publish setup instructions and the
original acquisition link instead. Do not include purchased or restricted assets
in the repository, installer, or sample packs. Locally prepared showcases are not part of the bundled samples.

## Asset provenance

The cube and directional-shadow scene are original ForgeSim content. The gallery
combines original ForgeSim content with credited third-party CC0 assets. The exact
model sources, modifications, and rights are recorded in
[`assets/models/README.md`](../apps/Sandbox/assets/models/README.md); environment
credits are in [`assets/environments/README.md`](../apps/Sandbox/assets/environments/README.md).
These existing assets are already included by the application packaging rules.

No new third-party content is introduced here. This change grants no additional
public license to original ForgeSim assets. Their existing rights statements
continue to apply; CC0 attribution of upstream components does not license the
whole gallery. Review permission for every asset before distributing any new pack.
