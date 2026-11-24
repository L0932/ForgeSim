# Scene loading measurements (#135)

The diagnostics extend `loading.diagnostics on`. A first optimization now shares
unchanged, self-contained GLB model resources across scene replacements. It does
not change asset quality or the persistent decoded-image cache format/policy.

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

## Image-stage breakdown

Completed image stages also report `Image work breakdown` with:

- `cache_setup`: cache initialization, lease acquisition, budget scan/trim.
- `encoded_key`: hashing encoded embedded bytes and constructing cache keys.
- `cache_read_validate`: cache lookup, pixel reads, checksum validation and recency
  updates; misses include the attempted lookup.
- `source_load_decode`: source image access, decompression and conversion to the
  loader's RGBA representation. Embedded bytes may already be in memory.
- `cache_write_publish`: cache mutex waits, checksum generation, file writes,
  eviction and publication. This is more than filesystem write time alone.
- `decoded_images`: how many images used the source decoder instead of cached pixels.
- `worker_limit`: configured concurrency limit for image batches, capped at four;
  actual concurrency can be lower for small image counts or final batches.

Only `cache_setup` is a serial interval. Other times sum each worker's operation
wall duration, including lock waits. They can overlap across workers and must not
be added to the overall image stage or interpreted as CPU utilization. Timings
still report when caching is disabled/unavailable; bypassed operations have zero
time and images are decoded normally. No cache format or decoding policy changes.

To measure uncached costs, close ForgeSim and move Cache aside before launching.
Keep caching enabled, then capture a first scene open. A same-session reopen now
reuses models and will not produce this breakdown for reused assets. To measure
warm disk-cache reads, restart the process with the populated cache retained.
Use the same build configuration and parallel-decode setting for both captures.

## Current reuse behavior

Scene Open still clears edit history and stages only the incoming scene's model
bindings. It can now borrow a previous model's immutable CPU meshes, materials,
images and GPU resources when the canonical model path, texture import policy,
file size and last-write time match. The GLB JSON is checked for external URIs;
only self-contained GLBs (including inline data URIs) qualify. Built-in cubes can
also be reused. JSON glTF and GLBs with external images/buffers keep the full
reload path, so external dependency edits cannot produce stale model reuse.

Failed or unavailable metadata checks bypass reuse. A source that changes during
import is not marked reusable. Invalid source imports still fail without replacing
the active scene's model bindings. Same-size edits that also preserve the original
last-write timestamp are not detected: restart the application to force a fresh
model import in that case. This is metadata validation, not content hashing.

`reused_models` counts distinct resources borrowed from the previous scene;
`reused_object_bindings` also includes repeated references in the incoming scene.
`preserve_history_resources` distinguishes an import from a scene replacement; it
does not indicate whether replacement-scene model sharing occurred.

Displacement variants are rebuilt on scene replacement, including external height
image sampling. Saved node overrides and per-object transforms remain applied to
that scene's objects; they are not stored in shared source meshes. Models absent
from the incoming scene are released after commit. There is no growing global
model cache. Old and newly loaded changed models can coexist temporarily while
staging, preserving rollback at the cost of transient peak memory. Existing import
and undo resource retention remains unchanged.

An unchanged environment path reuses its lighting resources as before; environment
content-change invalidation is not introduced by this patch. Disk image caching
still speeds process restarts; model sharing applies within one process only.

## Initial reference-scene observations

These are single user-captured runs before model reuse, not medians or a controlled
Release benchmark. Hardware, build configuration and OS-cache state have not yet
been supplied. The scene loaded 18 distinct model resources and reused one object
binding within that load. Decoded CPU image payloads total about 2.40 GiB; the
persistent image cache occupied about 2.27 GiB. These are different measurements,
not total process RAM or VRAM.

| Host measurement | Cache population | Restart, cache warm | Same-session reopen |
| --- | ---: | ---: | ---: |
| Scene open | 34.102 s | 5.766 s | 6.660 s |
| Image load/decode/cache phase | 30.370 s | 2.748 s | 2.338 s |
| Mesh processing | 0.888 s | 0.936 s | 0.722 s |
| Texture/material preparation | 1.632 s | 0.916 s | 2.588 s |
| Persistent image hits / misses | 2 / 57 | 59 / 0 | 59 / 0 |

These results motivate avoiding redundant resource preparation on reopen. They
do not measure the new optimization. Repeat the three scenarios after applying
model reuse, and inspect material appearance, object transforms, imported-node
overrides, shadows and picking. Also re-export a model, change texture policy,
and test a GLB with an external image to confirm full reload where required.

## Results to fill from the reference scene

| Metric (median; retain min/max) | Cache empty | Restart, cache warm | Same-session reopen |
| --- | --- | --- | --- |
| Scene open host time (ms) | pending | pending | pending |
| All model preparation (ms) | pending | pending | pending |
| Environment preparation (ms or reused) | pending | pending | pending |
| Resource commit/release (ms) | pending | pending | pending |
| Persistent image hits/misses | pending | pending | pending |
| Process peak RAM / cache disk bytes | pending | pending | pending |

The earlier approximately 28.7-second report is context, not a reproducible baseline
for this build. No speedup from the model-reuse patch is claimed yet. Use measured dominant stages to choose
the next change, then repeat all three scenarios and check rendering, transforms,
picking and imported-node overrides on the same scene.
