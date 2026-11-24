# ForgeSim

ForgeSim is an independently developed real-time simulation engine and editor
written in C++20, focused on interactive mechanical systems, equipment, and
training scenarios. Its Windows Sandbox combines OpenGL 4.6 rendering, scene
editing, asset inspection, and performance diagnostics in a docked workspace.

![Chess scene rendered in ForgeSim](docs/images/chess-scene.png)

*The bundled Chess Board sample rendered in ForgeSim under HDR environment lighting.*

[Build from source](#building-from-source-on-windows) ·
[Sample scenes](docs/SampleScenes.md) ·
[Keyboard controls](docs/KeyboardControls.md) ·
[Changelog](CHANGELOG.md) ·
[Roadmap](docs/Roadmap.md)

## Features

The Sandbox brings rendering, asset workflows, and scene editing together so you can build, inspect, and refine interactive scenes in one workspace.

### Scene editing

Build and arrange scenes through the docked editor, with viewport controls for direct manipulation and an Inspector for precise adjustments.

![ForgeSim editor displaying the Bistro scene](docs/images/bistro-outdoors-editor.png)

*An imported Bistro scene with its node hierarchy, per-instance transform controls,
and material inspection.*

- Docked Dear ImGui workspace with Hierarchy, Inspector, Scene, and Console panels.
- Model import, scene save/open, object duplication and deletion, and editable lights.
- Viewport selection, transform gizmos, local/world axes, and undo/redo.
- Imported glTF node hierarchies with per-instance node adjustments, subtree
  selection, focusing, and gizmos.
- Fly and orbit camera navigation with per-scene camera settings.
- Material texture inspection with channel views, zoom, panning, and pixel values.
- Per-scene viewport display settings, scene/editor PNG capture, and layout reset.
- Console commands with whole-log copying and individual **Copy line** actions.

See [keyboard controls](docs/KeyboardControls.md),
[imported model hierarchy](docs/ModelHierarchy.md), and
[the runtime Console](docs/runtime-console.md).

### Rendering

ForgeSim's forward renderer combines physically based materials, direct lighting, shadows, and HDR environment lighting to render imported glTF scenes.

![Textured cannon under outdoor environment lighting](docs/images/cannon-pbr.png)

*An imported cannon showing textured metal and wood under outdoor HDR lighting.*

- Base-color, metallic-roughness, normal, occlusion, and emissive textures,
  including supported UV texture transforms.
- Cook-Torrance directional, point, and spot lighting with editable light properties.
- Directional shadow mapping with filtering and bias controls.
- Diffuse irradiance and prefiltered specular image-based lighting.
- HDR equirectangular environments, configurable backgrounds, and vertical HDRI flipping.
- HDR rendering, exposure, ACES approximation or Standard/sRGB display output,
  and explicit linear/sRGB handling.
- Opaque, masked, blended, double-sided, and `KHR_materials_unlit` materials.
- Normal/tangent generation for supported missing source attributes and GLSL hot reloading.

See [display transforms](docs/DisplayTransforms.md),
[material texture previews](docs/MaterialTexturePreviews.md), and
[directional shadow validation](docs/DirectionalShadowValidation.md).

### Assets and displacement

Bring models into a project, inspect their materials and hierarchy, and add height-based surface detail through per-object displacement controls.

![Height-displaced material spheres in ForgeSim](docs/images/height-materials.png)

*Material spheres used to inspect height displacement, surface relief, and silhouette.*

- Load `.glb` and `.gltf` models with multiple meshes, primitives, materials, and
  authored node transforms.
- Discover project asset roots through `ForgeSim.project.json`, with explicit
  overrides and missing-asset recovery during editor scene opens.
- Choose HDR environments and height PNGs through native Windows file dialogs.
- Apply per-object/material CPU height displacement using linear 8/16-bit PNG
  sampling, bounded subdivision, independent height UV transforms, and optional
  smooth-seam stitching.
- Save applied height settings with scenes and restore them through undo/redo;
  source models remain unchanged.

Configure height displacement in the Inspector, or export displaced geometry
from Blender as a GLB. For Blender-baked meshes, leave ForgeSim displacement
disabled to preserve the exported shape.

See [project asset roots](docs/ProjectAssetRoots.md) and
[height displacement and Blender export](docs/prototypes/HeightDisplacement.md).

### Loading and diagnostics

ForgeSim speeds up scene loading by reusing loaded models and caching image data between sessions. Loading diagnostics help you understand how long a scene takes to open and where that time is spent.

To enable image caching and loading diagnostics:

```bash
./out/build/x64-Release/apps/Sandbox/ForgeSimSandbox.exe --loading-diagnostics --cache-decoded-images
```

Diagnostics report document loading, model preparation, image/cache work,
environment preparation, and scene-open completion using host timings.

The cache normally resides at
`%LOCALAPPDATA%/ForgeSim/Cache/decoded-images-v2/` on Windows. The configurable
budget controls storage for the active cache version.

See [scene-loading performance](docs/performance/SceneLoading.md) for measured
results, cache behavior, timing definitions, and repeatable benchmark steps.

## Try the Sandbox

Download an available Windows package from [Releases](https://github.com/L0932/ForgeSim/releases),
or build from source below. For a portable package, extract the complete folder
and keep `assets`, `ForgeSim.project.json`, and dependency notices with the executable.
An OpenGL 4.6-capable graphics driver is required.

Startup and **New Scene** open an empty workspace with a reference grid,
directional light, neutral background, and studio environment lighting.

Choose **File > Open Sample**:

| Bundled sample | What to explore |
| --- | --- |
| Starter: Directional Shadows | Transform cubes and inspect shadows on a solid floor. |
| Material Gallery | Compare material factors, texture inputs, and roughness. |
| Chess Board | Inspect imported pieces and a table under an interior HDRI. |

Samples include their required assets and open as editable copies. Save asks for
your own destination. External showcase assets are obtained separately; the
sample guide records bundled content and asset credits.

See [sample scenes and asset credits](docs/SampleScenes.md) and
[running the installed application](docs/InstalledApplication.md).

## Use your own assets

Pass a model path to import it at launch:

```bash
./out/build/x64-Release/apps/Sandbox/ForgeSimSandbox.exe "C:/MyProject/models/Example.glb"
```

Open a saved ForgeSim scene with `--scene`:

```bash
./out/build/x64-Release/apps/Sandbox/ForgeSimSandbox.exe --scene "C:/MyProject/scenes/Example.scene.json"
```

Place a `ForgeSim.project.json` manifest in the project folder:

```json
{
  "version": 1,
  "assetRoot": "."
}
```

Keep scenes beneath that folder and store model/environment references relative
to the declared asset root, such as `models/Example.glb`. ForgeSim searches upward
from a scene for the nearest manifest. Standalone model imports without a manifest
use the model's containing folder. Save scenes and their referenced assets together
under the project folder for portability. Create the manifest manually; scene files
store asset references rather than embedding the assets.

Use `--project-root "C:/MyProject"` for an explicit root override. In the Console,
`project.root.auto` clears a session override and restores discovery for future
editor opens. See [project asset roots](docs/ProjectAssetRoots.md) for saved sample
copies, fallback behavior, and asset recovery.

Useful launch options:

| Option | Purpose |
| --- | --- |
| `--allow-texture-fallbacks` | Skip unusable texture/UV bindings and retain material factors. |
| `--cache-decoded-images` | Enable the decoded-image disk cache. |
| `--image-cache-budget-mib 1024` | Set a session cache budget; requires caching enabled. |
| `--loading-diagnostics` | Log import progress and loading-stage timings. |
| `--serial-image-decode` | Decode images serially for comparisons. |
| `--help` | Show supported arguments. |

Console commands `images.cache`, `loading.diagnostics`, and `textures.fallback`
accept `on` or `off`, or report the current setting without an argument. They apply
to subsequent loads; changing a setting does not reload the active model.

For large Intel Sponza assets, [Prepare-SponzaTextures.ps1](tools/Prepare-SponzaTextures.ps1)
can create a lower-resolution development copy using ImageMagick. The source
assets are obtained separately.

## Building from source on Windows

Use Git, CMake **3.24 or later**, Ninja, and an x64 MSVC toolchain with C++20 support.
Run commands from a terminal with the x64 MSVC developer environment active.
Check `git --version`, `cmake --version`, `ninja --version`, and `cl` first.

Initial configuration downloads dependencies through CMake FetchContent. GLAD's
generated source is included; generating it does not require Python or Jinja2.

```bash
git clone https://github.com/L0932/ForgeSim.git
cd ForgeSim
```

Configure, build, test, and launch Debug:

```bash
cmake -S . -B out/build/x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x64-Debug --parallel
ctest --test-dir out/build/x64-Debug --output-on-failure
./out/build/x64-Debug/apps/Sandbox/ForgeSimSandbox.exe
```

For Release:

```bash
cmake -S . -B out/build/x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/x64-Release --parallel
ctest --test-dir out/build/x64-Release --output-on-failure
./out/build/x64-Release/apps/Sandbox/ForgeSimSandbox.exe
```

The test suite covers core timing, asset processing, scene data, selection,
transforms, edit history, displacement, and cache behavior.

PNG acceleration is enabled by default. Configure with
`-DFORGESIM_ACCELERATE_PNG=OFF` for the stb-only path. Displacement command-line
tools are optional; see the [prototype guide](docs/prototypes/HeightDisplacement.md).
Package preparation and validation are documented in [Releasing](docs/Releasing.md).

## Documentation

The guides below cover everyday editor workflows, rendering and asset behavior, and the architecture behind ForgeSim.

| Area | Guides |
| --- | --- |
| Editor | [Keyboard controls](docs/KeyboardControls.md), [camera navigation](docs/CameraNavigation.md), [screenshots](docs/Screenshots.md) |
| Imported models | [Hierarchy and node edits](docs/ModelHierarchy.md), [transforms](docs/ImportedTransforms.md), [texture transforms](docs/TextureTransforms.md) |
| Materials | [Inspection](docs/MaterialInspection.md), [texture previews](docs/MaterialTexturePreviews.md), [display transforms](docs/DisplayTransforms.md), [unlit materials](docs/UnlitMaterials.md) |
| Projects and samples | [Asset roots](docs/ProjectAssetRoots.md), [sample scenes](docs/SampleScenes.md), [installed application](docs/InstalledApplication.md) |
| Diagnostics | [Runtime Console](docs/runtime-console.md), [scene-loading performance](docs/performance/SceneLoading.md) |
| Development | [Architecture](docs/Architecture.md), [roadmap](docs/Roadmap.md), [changelog](CHANGELOG.md), [release process](docs/Releasing.md) |

## Direction and development approach

The longer-term goal is reusable infrastructure for mechanical assemblies,
component interaction and failure, equipment operation, and training scenarios.
Scenario systems and future immersive/VR interaction will build on the current
scene, rendering, and editor foundation.

ForgeSim combines fixed-timestep scheduling with bounded catch-up, reusable engine
modules, and editor tooling. See the roadmap for upcoming simulation and workflow
features.

AI-assisted tools may be used for research, review, validation, and implementation
assistance. All committed material remains the project owner's responsibility and
must be understood, reviewed, tested, and maintainable. Generated output is subject
to the same verification requirements as other contributions.

## Independent Development

ForgeSim is a personal project developed independently of any employer, client, or contracting engagement.

The project's design, planning, architecture, source code, documentation, and other repository materials are developed using personal resources and maintained separately from work performed for employers or clients.

No confidential information, proprietary material, source code, datasets, work product, or other restricted materials belonging to an employer or client are intended to be incorporated into this repository.

## Repository History

The repository's Git history records the ongoing implementation and evolution of ForgeSim.

Architecture and implementation decisions may change as the project develops. The Git history should be considered the authoritative development record for material committed to the repository.

## Ownership

Unless otherwise explicitly stated, ForgeSim and original material contained in this repository are independently authored personal work.

Third-party libraries and dependencies remain subject to their respective licenses.

### Licensing

Copyright © 2026 Arnold Lopez. All rights reserved.

The source code is publicly available for viewing and evaluation. No general license to copy, modify, redistribute, or commercially exploit the original ForgeSim code is granted at this time.

Third-party components remain subject to their respective licenses.

A formal licensing model will be announced in the future.
