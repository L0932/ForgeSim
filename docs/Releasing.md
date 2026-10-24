# ForgeSim Release Conventions

## Version Source

The numeric application version is defined by VERSION in the root
CMakeLists.txt project() declaration.

FORGESIM_VERSION_PRERELEASE supplies an optional prerelease identifier.
It defaults to dev.

Examples:

- 0.1.0-dev: ongoing development
- 0.1.0-preview.1: a preview for evaluation
- 0.1.0: a published release without a prerelease suffix

Application diagnostics and future package configuration should derive
their versions from these values rather than duplicate them.

## Numbering

During initial development, ForgeSim uses 0.MINOR.PATCH.

- Increment MINOR for a significant feature release or incompatible change.
- Increment PATCH for compatible fixes and small maintenance updates.
- Increment the preview number for another preview of the same planned release.

Version 1.0.0 will identify an explicitly defined supported baseline.
Completing a roadmap phase does not automatically determine a version.

Document breaking changes and compatibility limitations in release notes.

## Changelog

Maintain CHANGELOG.md with an Unreleased section.

Before publishing a release:

1. Review the changes since the previous release.
2. Summarize user-visible additions, changes, fixes, and breaking changes.
3. Move the applicable entries into a dated version section.
4. Leave an Unreleased section for subsequent development.

Do not treat every commit as a changelog entry.

## Tags

Release tags use v followed by the complete application version.

Examples:

- v0.1.0-preview.1
- v0.1.0

Create tags only after completing the release's required validation.
Normal development builds do not require tags.

## Scene Documents

Scene-document schema versions are independent of application versions.

Change a schema version only when required by the document compatibility
contract. Application version changes do not automatically change schemas.

## Distribution

Packaging and automated publishing are separate infrastructure tasks.

Published packages must include required runtime resources, applicable
third-party notices, and only assets permitted for redistribution.