# Changelog

This file records meaningful changes in published ForgeSim releases.

Changes under Unreleased have not yet been published. Git history
contains the detailed implementation record.

## [Unreleased]

### Added

- Mouse-wheel fly-speed adjustment during right-mouse camera navigation.

- Per-scene fly-speed persistence with an Automatic fly speed option, independent
  of explicit viewpoint capture.

- Camera fly-speed and near/far clipping controls, Ctrl precision movement,
  and scene/selected-object framing actions in the Camera panel.

- Bundled directional-shadow starter and material-gallery scenes, available from
  **File > Open Sample** without project-root configuration.
- Samples browser with screenshot previews and local scene opening using
  automatic asset-root discovery.
- Editable sample copies that ask for a save destination and preserve bundled
  scene documents.
- Sample setup, asset provenance, and distribution guidance included in the
  application package, with package-relative credit links and its build version.

- Project and sample-pack manifests with portable asset roots, shared discovery
  for editor and command-line scene opens, and a bundled application manifest.
- `project.root.auto` to clear explicit root overrides for subsequent scene opens.
- Missing-asset recovery with model and folder pickers, retry, and cancellation.
- A checked-by-default texture-fallback option for local sample opens, without
  changing the global texture policy.

### Fixed

- Automatic camera speed and near clipping are more conservative for large
  imported environments, while retaining scale-aware defaults.

- Direct external model imports discover their project manifest, or use the
  model folder when none exists, avoiding invalid scene references after GPU
  loading. Incompatible explicit roots are rejected before loading.

- Unused source materials no longer cause GPU upload failures after texture
  fallback creates compatible material variants for the actual mesh primitives.

- Reset Layout now restores the Samples window's default size and position,
  including clearing saved placement when the window is closed.

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
