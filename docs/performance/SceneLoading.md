# Scene loading measurements (#135)

This first measurement pass extends `loading.diagnostics on`. It does not change
cache policy, asset quality, or resource ownership. Performance targets and an
optimization will be selected after measuring the reference scene.

## Capture a baseline

Use the same Release build, scene, camera and application settings for every run.
Record commit, CPU, GPU, RAM, storage device, driver, scene/model/image sizes,
unique model count, object count, texture fallback policy, parallel image decode
setting, decoded-image cache budget and whether diagnostics are enabled. Keep the
same diagnostics setting when comparing builds. Do not compare Debug to Release.

Enable the existing console commands before opening the reference scene:

```text
loading.diagnostics on
images.cache on
```

Measure at least three runs of each scenario and retain the complete logs:

| Scenario | Procedure | What it measures |
| --- | --- | --- |
| ForgeSim cache empty | Close ForgeSim. Move its Cache directory aside, launch, enable diagnostics/cache, then open the scene. Clear the generated cache again before each repeat. | Import without ForgeSim persistent image-cache hits. |
| Persistent cache warm | Populate the cache once, close ForgeSim, relaunch with the same settings, then open the scene. | Persistent decoded-image reuse across process restarts. |
| Same-session reopen | After a successful load, reopen the same scene in that process. | Current reopen behavior, including any environment reuse. |

The Cache directory is alongside Settings under the ForgeSim user-data directory;
use the logged layout/settings location to find it. On Windows this normally
means `%LOCALAPPDATA%/ForgeSim/Cache`. Move only Cache while the app is closed;
retain scenes and settings. Restore the saved directory afterward if desired.
Turning `images.cache off` bypasses the cache; it does not clear existing entries.
An empty ForgeSim cache does not imply an empty operating-system file cache.
Record OS-cache state as uncontrolled unless separately managed.

## Interpret the logs

- `Scene open completed` is the host duration through scene activation. It excludes
  time spent choosing a file and does not guarantee presentation of the first
  completed scene frame or completion of all GPU work.
- `Scene document loaded` and `Scene resources ready` are child intervals of scene
  open. Their difference from the total includes final editor preparation.
- `Loading model resources` identifies the asset associated with subsequent
  resource-stage messages. Existing CPU import logs include document parsing,
  buffer loading, validation, image decode/cache activity and mesh processing.
- `model total` contains CPU import, bounds, upload preparation and diagnostic
  logging for that model. Mesh-buffer and texture/material intervals are separate
  child phases; textures include mipmap generation calls.
- `all models including height and bounds` includes per-object displacement and
  bounds preparation. It is not another cost to add to the model totals.
- Environment messages split source image load/upload, cubemap conversion,
  diffuse irradiance and specular prefilter submission. The environment total
  contains those child phases. An unchanged path logs environment reuse.
- `resource commit and release` includes replacing bindings and releasing obsolete
  resources. `scene resources total` contains model preparation, environment work
  and commit/release. Do not sum parent and child durations.
- All new times use `steady_clock` on the host. OpenGL may queue work or block in
  later calls. These are not GPU timer-query measurements; no `glFinish` is added
  to alter the baseline. Progress drawing and logging contribute to wall time.
- CPU payload bytes count mesh vertex/index elements and decoded image pixels.
  They exclude allocation capacity, source documents, embedded encoded images,
  temporary decoder buffers, driver allocations and GPU texture storage. They are
  not peak RAM or VRAM measurements. Record process peak memory separately.
- Resource counts identify actual uploaded primitive/texture/material objects.
  `reused_object_bindings` counts objects served by already loaded resources,
  including repeated references within this load; it is not a disk-cache hit count.
  Existing decoded-image logs separately report persistent cache hits/misses.

## Current reuse behavior

Scene Open explicitly sets `reuseLoadedModels=false`: each distinct model is
imported/uploaded again on reopen, while duplicate references within that scene
share resources. Importing additional objects can reuse existing models. An
unchanged environment path reuses its current lighting resources.

Do not enable cross-scene model reuse using only a path comparison. An optimization
must account for changed model/external dependencies, import settings and texture
policy, and preserve failure rollback, material boundaries, node identity and
saved overrides. Existing image-cache tests cover their own cache behavior; this
measurement patch does not introduce a processed-mesh cache.

## Results to fill from the reference scene

| Metric (median; retain min/max) | Cache empty | Restart, cache warm | Same-session reopen |
| --- | --- | --- | --- |
| Scene open host time (ms) | pending | pending | pending |
| All model preparation (ms) | pending | pending | pending |
| Environment preparation (ms or reused) | pending | pending | pending |
| Resource commit/release (ms) | pending | pending | pending |
| Persistent image hits/misses | pending | pending | pending |
| Process peak RAM / cache disk bytes | pending | pending | pending |

The reported approximately 28.7-second load is context, not a reproducible baseline
for this build. No speedup is claimed yet. Use measured dominant stages to choose
the next change, then repeat all three scenarios and check rendering, transforms,
picking and imported-node overrides on the same scene.
