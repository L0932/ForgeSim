# Changelog

This file records meaningful changes in published ForgeSim releases.

Changes under Unreleased have not yet been published. Git history
contains the detailed implementation record.

## [Unreleased]

## [0.1.0-preview.9] - 2026-11-12

### Added

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
