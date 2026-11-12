# ForgeSim Sample Scenes

## Open a bundled sample

Build the Sandbox or extract the complete official application package. In the
editor, choose **File > Open Sample**:

| Sample | Purpose | Required bundled assets |
| --- | --- | --- |
| Starter: Directional Shadows | Select and transform cubes, toggle visibility, and inspect directional shadows on a floor. | `ShadowTestCube.gltf` |
| Material Gallery | Compare metallic and nonmetallic materials across roughness values. | `PbrMaterialGallery.glb` |

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

## Source checkout

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
in the repository, installer, or sample packs. Locally prepared showcases such as
Cathedral are not part of the bundled samples.

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
