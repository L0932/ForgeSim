# Changelog

This file records meaningful changes in published ForgeSim releases.

Changes under Unreleased have not yet been published. Git history
contains the detailed implementation record.

## [Unreleased]

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