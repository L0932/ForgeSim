# Changelog

This file records meaningful changes in published ForgeSim releases.

Changes under Unreleased have not yet been published. Git history
contains the detailed implementation record.

## [Unreleased]

### Fixed

- Reuse unchanged self-contained GLB model resources across scene replacements,
  avoiding repeat image reads, mesh processing and GPU uploads. Validate source
  size/time and texture policy; externally referenced assets retain full reloads.
  Rebuild per-scene displacement and retain saved node overrides (#135).

- Keep the HDR image picker folder history separate from Open Scene.
- Remove shadowed bounds variables in orbit controls, resolving MSVC warning C4456.

### Added

- Add a Console Copy log button for copying retained messages and commands to
  the clipboard, including entries hidden by severity or text filters.

- Expand optional scene-loading diagnostics with per-model resource timings,
  mesh/texture upload preparation, environment stages, reuse counts and CPU
  payload sizes. Add a repeatable three-scenario loading benchmark guide (#135).

- CPU height displacement with linear 8/16-bit PNG sampling, bounded triangle
  subdivision, rebuilt normals/tangents, and optional smooth-seam stitching.
- Per-object material height controls with embedded/external PNG selection,
  independent height UV mapping, undo/redo, duplication, and scene persistence.
  External height images are copied into the project without overwriting files.
- Opt-in displacement tools for flat/displaced comparison scenes and baking static
  GLB geometry while preserving source materials and textures. Document both
  ForgeSim height overrides and Blender modifier-based GLB export (#132).
- Native Windows HDR environment image picker with active asset-root display.
  Images outside the asset root are copied into its environments folder without
  overwriting existing files; selected environments save as portable references.

## [0.1.0-preview.11] - 2026-11-21

### Fixed

- Keep Ctrl+F object orbit and the Camera checkbox synchronized through the same
  saved orbit action.
- Save viewport grid, light guide, bounds, selection highlight and transform gizmo
  visibility per scene, restoring defaults for older files and new scenes.
- Correct split-sum environment specular lighting to use normal-incidence
  reflectance, avoiding duplicate Fresnel brightening at grazing view angles.
- Correct normal-map bitangent handedness for glTF meshes without tangents and
  for transformed normal-map UVs, preserving authored tangent attributes.
- Correct Inspector base-color and emissive previews using independent source-color
  textures. Add packed RGB, roughness (G), and metallic (B) views without changing
  rendering textures, source images, or material factors.
- Startup and New Scene now share an empty workspace with a reference grid,
  directional light, neutral background, studio environment lighting, and an angled
  camera with a 0.1 near clip. The Material Gallery remains available through Samples.
  Empty scene startup no longer loads an unused gallery model.
- Creating a new scene restores default viewport visibility, including the reference
  grid, scene light guides, selection highlight and transform gizmo.

### Added

- A scene-persistent Camera checkbox for orbiting one selected showcase object,
  restoring its target, viewpoint and Orbit mode hint when opened.
- Separate GUI controls for selection highlighting and transform gizmo visibility,
  with Shift+H retained as a combined hide/show shortcut.
- Shift+H toggles the selection tint and transform gizmo together,
  keeping the Inspector selection active. It works over viewport content.
- Scene-persistent Standard/sRGB display output alongside the default ACES
  approximation, with exposure retained and HDR highlights clipped for comparison.
- Full-resolution Inspector texture inspection with fit, 1:1, pointer-centered
  zoom, drag panning, and source pixel values for comparing exported texture data.
  Fit follows window resizing; manual zoom and pan remain stable until Fit is selected.
- Support glTF `KHR_materials_unlit` materials, including required extension
  declarations, base-color textures, vertex colors and existing alpha modes.

## [0.1.0-preview.10] - 2026-11-16

### Added

- Bundled Chess Board sample with chess pieces, a wooden tea table, Thatch
  Chapel HDRI, saved viewpoint, node adjustments, preview image, and packaged
  Poly Haven asset attribution.
- Scene and full-editor PNG screenshots through Capture, F9, and Ctrl+F9,
  with an output-folder picker, unique filenames, and capture feedback.
- Shift+Space maximizes or restores Scene while preserving the dock layout.
- Per-scene vertical HDRI flipping, applied consistently to the environment
  background, diffuse lighting, and specular reflections.
- Persistent editor preferences for reference grid, scene light guides,
  and object bounds visibility across scene changes and application restarts.
- Imported model node selection through Hierarchy or Alt-click geometry,
  with subtree highlighting, focus, and inspection of local transforms,
  world bounds, mesh references, and associated materials.
- Per-instance node transform adjustments through Inspector and scene gizmos,
  including translation, rotation, local-axis scale, world/local axes, Reset,
  undo/redo, and version 5 scene persistence. Rendering, shadows, picking,
  bounds, and focus follow the adjusted hierarchy. Model instances retain
  independent adjustments without changing shared source assets.
- Toggle selection focus and the previous camera view with F, Hierarchy label
  double-clicks, or Camera controls. Supports objects, groups, node subtrees,
  and lights.
- Ctrl+F selection orbit with RMB-drag rotation, wheel zoom, Camera controls,
  and an on-screen navigation hint. Exiting orbit can restore the previous view.
- Shift+H toggles selection highlighting without clearing the selection.

### Changed

- Startup loads the bundled Material Gallery scene with its saved camera,
  navigation speed, lighting, and rendering settings.
- Group environment background visibility and vertical flipping under
  Environment asset.
- Centralize JSON schema-version constants for editor preferences and
  project manifests.

### Fixed

- Refresh model bounds immediately after node undo/redo so model picking
  and directional shadow fitting use the current geometry.

## [0.1.0-preview.9] - 2026-11-12

### Added

- Ctrl/Shift multi-selection for model instances in Scene and Hierarchy, with
  group highlighting, focus, transforms, pivots, visibility, deletion and
  duplication. Group edits support one-step undo/redo; Inspector values and
  materials describe the active member. Internal glTF nodes remain read-only.
- Expand model instances in Hierarchy to browse read-only glTF nodes, meshes,
  primitive counts, and material references.
- Scene authoring controls: GameObject menus for built-in cubes and directional,
  point, and spot lights; deletion with Delete; and duplication with Ctrl+D for
  model instances, cubes, point lights, and spot lights. Creation, deletion, and
  duplication support undo/redo, selection, visibility, and scene persistence.
- Read-only Inspector material values and texture-map previews with dimensions,
  UV sets, packed-channel descriptions, hover previews, responsive columns, and
  a compact summary of unassigned maps.
- Searchable Help > Keyboard Controls reference and context-aware F focus for
  selected model instances and lights, shared with the Camera-panel action.
- Import glTF models into the active scene for side-by-side comparisons, with
  independent model materials, bounds, selection, and save/reopen support.
- Camera fly-speed and near/far clipping controls, mouse-wheel speed adjustment
  during right-mouse navigation, Ctrl precision movement, and scene framing.
- Per-scene fly-speed persistence, independent of explicit viewpoint capture.
- Bundled directional-shadow starter and material-gallery scenes, available from
  File > Open Sample without project-root configuration.
- Samples browser with screenshot previews, editable sample copies, and local
  scene opening using automatic asset-root discovery.
- Sample setup, asset provenance, and distribution guidance in the application
  package, with package-relative credit links and its build version.
- Project and sample-pack manifests with portable asset roots, shared discovery
  for editor and command-line opens, and a bundled application manifest.
- `project.root.auto` to clear explicit root overrides for subsequent scene opens.
- Missing-asset recovery with model and folder pickers, retry, and cancellation.
- A checked-by-default texture-fallback option for local sample opens, without
  changing the global texture policy.

### Changed

- Default camera fly speed to 10 units/s for new scenes and files without saved
  navigation speed; replace automatic speed with a reset-to-10 action.
- Allow directional lights to be removed entirely, saved as absent, and recreated.
- Place the default directional light above the grid in new scenes.
- Number duplicated object names instead of accumulating Copy suffixes.
- Identify saved scenes by filename in the toolbar and load/save messages.
- Remember model-import and missing-model dialog folders independently from
  scene Open and Save As across Windows sessions.

### Fixed

- Reset camera navigation from the active scene after New, Open, and model import,
  including when viewpoint restoration bypasses automatic framing.
- Use workspace clipping defaults for empty scenes so the reference grid is not
  cut off by placeholder bounds. Give empty-scene light guides and picking a
  consistent workspace size after New/Open.
- Fit selection focus to the current view for tighter framing of long, flat models;
  retain scene-wide far clipping after focusing lights or small objects.
- Direct external model imports discover their project manifest, or use the
  model folder when none exists, avoiding invalid scene references after GPU
  loading. Incompatible explicit roots are rejected before loading.
- Unused source materials no longer cause GPU upload failures after texture
  fallback creates compatible material variants for actual mesh primitives.
- Reset Layout restores the Samples window's default size and position,
  including clearing saved placement when the window is closed.
- Windows asset-root tests compare filesystem identity rather than path spelling.
- Link light validation into scene-edit-history tests and remove a shadowed
  local bounds variable in the model-import path.

## [0.1.0-preview.2] - 2026-10-26

### Added

- Windows installer generated through CPack NSIS.
- Start Menu shortcuts and application uninstallation.
- Detection of existing installations when installing a replacement version.
- GitHub Actions validation for Windows Debug and Release builds and headless tests.

### Fixed

- Scene-save path tests now compare filesystem identity rather than path spelling.

## [0.1.0-preview.1] - 2026-10-24

### Added

- Relocatable Sandbox installation with bundled resources and compiler runtime dependencies.
- User-data directories for editor settings and default scene saves.
- Portable Windows ZIP generation through CPack with a SHA-256 checksum.
- Installation instructions, release notes, and third-party notices.
- Application version information generated from CMake configuration.
- Version reporting in startup diagnostics and the Sandbox debug panel.
- Release numbering and tagging conventions documented in docs/Releasing.md.
