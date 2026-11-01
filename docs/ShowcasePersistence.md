# Reproducing a showcase view

Scene document version 2 stores an optional active `camera`, a `cameraBookmarks`
array, and `rendering` settings. Saving the scene writes these authored settings.
The selected object, grid and light guides, selection highlight, editor layout,
window size, and navigation speed remain editor state.

## Camera and bookmarks

- **Save current viewpoint** captures position, forward/up directions (including
  roll), vertical FOV, clipping distances, and the current Scene viewport's physical
  pixel dimensions. Save the scene file to persist that snapshot.
- Ordinary WASD/mouse navigation never updates the authored snapshot.
- **Clear saved viewpoint** restores automatic framing the next time the scene is
  opened. It does not delete bookmarks or immediately move the working camera.
- Give a bookmark a unique name and choose **Add bookmark** to capture the working
  camera. Names are case-sensitive, contain 1–128 UTF-8 bytes, and may not contain
  control characters or consist entirely of spaces.
- Select a bookmark in **Bookmarks** to restore its pose/projection and make it
  the scene's saved active viewpoint. Selection can therefore mark the scene dirty.
- **Update bookmark from current view** changes that bookmark's snapshot. Select
  it again to make the updated snapshot active. The saved active camera is an
  independent snapshot, so deleting a bookmark does not lose the active viewpoint.
- **Delete bookmark** removes the selected bookmark. Save the scene afterward.

Reference dimensions are metadata, not a request to resize the editor. The camera
uses the current viewport aspect ratio. Match the recorded Scene viewport width
and height (not the outer window dimensions) for reference captures. A differing
size produces a notice in the debug panel. Match resolution, camera, scene,
lighting, environment and rendering settings when comparing images. GPU/driver
variations can still produce small visual differences.

## Presentation and environments

Exposure, fallback background color, environment intensity/background visibility,
and all implemented directional shadow parameters belong to the scene. Shadow
reset restores the four default shadow values and participates in dirty detection.

Environment sources:

| Source | Saved reference | Behavior |
| --- | --- | --- |
| Bundled studio | `bundledStudio`, asset `null` | Uses the installed `assets/environments/Studio_Small_01_4k.hdr` |
| None | `none`, asset `null` | Black environment; no environment-map lighting |
| Project HDR asset | `projectAsset`, project-relative asset string | Loads a Radiance HDR image and rebuilds its IBL maps |

For example, with project root `C:/ForgeSimAssets`, type
`environments/Cathedral.hdr` and choose **Apply project HDR**. The file resolves to
`C:/ForgeSimAssets/environments/Cathedral.hdr`, independent of the scene-file
location. Absolute paths, drive prefixes, backslashes, repeated slashes and `.` or
`..` components are rejected using the same rules as model references. Paths are
stored as UTF-8. Keep referenced assets available when moving the project.

A missing file is rejected during scene save/load. HDR decoding and GPU map
creation happen before the working environment is replaced. When opening a scene,
model and environment preparation both finish before committing the new session.
Failed preparation leaves the active session and renderer resources intact. IBL
preprocessing is synchronous and can briefly pause the editor. The BRDF lookup
texture is reused because it does not depend on the chosen environment.

Environment choice and exposure are separate: disabling the background does not
disable environment lighting. **None** removes IBL, while directional/point/spot
lighting and emissive surfaces continue to work.

## Compatibility and validation

- Version 1 files migrate to version 2 and retain automatic framing.
- Earlier version 2 files without rendering data use the existing appearance
  defaults: exposure `1`, environment intensity `0.2`, hidden environment
  background, fallback RGB `(0.032, 0.036, 0.044)` and bundled studio environment.
- Missing `directionalShadows` uses enabled shadows/PCF, minimum bias `0.0005`
  and slope bias `0.0025`. Missing bookmarks means an empty list.
- Old camera objects without `referenceViewport` keep that metadata absent.
- New optional fields remain compatible additions to version 2. If an object is
  present, its required members and types are checked; malformed objects do not
  silently fall back to defaults.
- Minimum bias must be finite in `[0, 0.01]`; slope bias in `[0, 0.02]`, including
  when shadows are disabled. Rendering and camera errors identify the property.
- The current target-based camera rejects positions where a unit direction can
  no longer be represented as a distinct floating-point target.

## Windows validation

From the repository root, build every target and run all tests:

```bash
cmake --build out/build/x64-Debug --config Debug
ctest --test-dir out/build/x64-Debug -C Debug --output-on-failure --no-tests=error
```

Repeat using the configured Release build directory and `-C Release`. If using
one Visual Studio multi-configuration build tree, keep that tree and change only
the configuration. The new focused test is `ForgeSim.Sandbox.ShowcasePersistence`.

Manual checks (require a running OpenGL application):

1. Load a scene, add two distinct bookmarks, update one, and select each. Confirm
   pose/projection restoration and continued WASD navigation. Save/reopen and
   confirm the active viewpoint and named list survive.
2. Delete a selected bookmark. Confirm the active camera remains valid. Clear the
   active viewpoint, save/reopen, and confirm automatic framing is used.
3. Toggle shadows and PCF, edit both biases, save/reopen and inspect the controls
   and output. Reset shadow settings and confirm the scene becomes modified.
4. Switch between bundled studio, None and a project HDR. Check reflections,
   environment background, and intensity. Save/reopen each choice.
5. Apply a nonexistent HDR, then an existing non-HDR file. Confirm a useful error,
   unchanged authored settings, and unchanged environment/model appearance.
6. Open a scene referencing the invalid HDR while another scene is active.
   Confirm the existing session/model/environment remain usable.
7. Resize the Scene viewport and confirm the reference-dimension notice appears.
   Match its recorded dimensions before taking a reference capture.
8. Create a new scene. Confirm bookmarks/viewpoint are empty and appearance uses
   defaults, while personal debug-overlay choices remain independent.

The headless suite covers serialization, validation, bookmark edits/selection,
projection restoration, reference dimensions, dirty detection, environment path
resolution, missing references, failed saves/opens, and legacy defaults. GPU HDR
conversion, visual rendering, ImGui interaction and Windows-specific file behavior
must also pass the manual checks above.
