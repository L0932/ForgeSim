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

Application diagnostics and package configuration derive their versions
from these values rather than duplicate them.

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

Release preparation is automated. Public publication remains a deliberate
maintainer action.

Published packages must include required runtime resources, applicable
third-party notices, and only assets permitted for redistribution.

## Automated Release Preparation

The Prepare Release workflow is started manually from GitHub Actions.
Run it on main and supply a candidate tag such as v0.1.0-preview.3.

The tag's numeric version must match project(VERSION ...) in the root
CMakeLists.txt. Its prerelease suffix is supplied to CMake automatically.
The source default can remain dev during normal development.

The workflow:

1. Validates the candidate tag and application version.
2. Builds the Windows x64 Release configuration.
3. Runs the headless tests through CTest.
4. Creates the portable ZIP and NSIS installer through CPack.
5. Generates and verifies SHA-256 checksum files.
6. Uploads the packages as workflow artifacts.
7. Creates or updates a draft GitHub release with the four files attached.

A failed build, test, or packaging check prevents draft preparation.
Only the draft-release job has repository write permission.

Workflow artifacts expire after 14 days. Files attached to a GitHub
release are separate from those temporary artifacts.

## Preparing a Candidate

1. Review the changes intended for the release.
2. Update CHANGELOG.md with the appropriate version section.
3. Commit and push the intended source and documentation to main.
4. Open Actions > Prepare Release > Run workflow.
5. Select main and enter the complete candidate tag.
6. Confirm both workflow jobs succeed.
7. Open the draft release and review its source commit and assets.

The workflow records the exact commit used for the build. Later commits
on main do not change an already prepared candidate.

Generated release notes are a starting point. Edit the draft to explain
meaningful changes, known limitations, requirements, and installation.

## Manual Validation Before Publication

Download the packages attached to the draft release.

- Verify both package checksums after downloading.
- Extract the portable ZIP into a fresh directory and launch the Sandbox.
- Check the reported application version and bundled material gallery.
- Check rendering, camera navigation, selection, and transform editing.
- Check undo/redo and scene save/reopen behavior.
- Install the application and verify its shortcut and launch behavior.
- Verify replacement installation and uninstallation.
- Check bundled resources and third-party notices.
- Confirm purchased assets and personal user data are absent.

Headless tests do not replace interactive and visual validation.

Publish the draft manually only after completing these checks and
reviewing the final release notes. Preview releases must retain the
prerelease designation.

## Retries and Corrections

Use Re-run all jobs on the original workflow run to retry preparation
from the same source commit.

Retries update the existing draft's matching assets without creating
duplicates. Existing draft notes are preserved.

If source changes are required, use a new candidate version. Do not move
an existing release tag or reuse a published version for different files.

The workflow rejects existing tags pointing to another commit, drafts
targeting another commit, and published releases.

Normal development does not require running Prepare Release or setting
FORGESIM_EXPECTED_VERSION. That CMake setting is an optional release
guard; ordinary build directories should leave it empty.