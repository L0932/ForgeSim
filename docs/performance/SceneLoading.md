# Scene loading measurements (#135)

The diagnostics extend `loading.diagnostics on`. Model reuse shares
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

## Detailed cache intervals

`Image cache detail` reports worker-summed child intervals of the existing cache
read/write totals:

- `mutex_wait`: time acquiring the model cache's mutex for lookups, recency updates
  and writes. Workers may wait while another worker stages/publishes a write.
- `read_io_allocate`: opening and validating the cache header/length, allocating
  the decoded pixel buffer, reading pixels and closing the file. This is not a
  disk-only counter: allocation, OS cache hits and filesystem metadata are included.
- `read_checksum`: hashing decoded pixels and comparing the stored checksum.
- `write_checksum`: hashing decoded pixels for a new cache entry.
- `write_io`: opening the staging file, writing its header/pixels and closing it.
  Closing does not imply durable storage flush; OS buffering still applies.
- `publish_evict`: budget eviction and publishing the staging file.

Do not add these values to the parent cache totals or scene-open wall time.
Worker intervals can overlap, and minor control/metadata work remains outside
these detailed intervals. The cache format and locking rules are described below.

Capture an empty-cache Release load to measure writes, then restart with the
populated cache to measure reads. Same-session model reuse bypasses both paths.

## Version 2 cache integrity and write locking

Decoded pixels now use IEEE CRC-32 instead of byte-at-a-time FNV-1a. The existing
libdeflate dependency supplies an accelerated implementation when PNG acceleration
is enabled; the stb-only build uses a matching portable implementation. Both builds
can read each other's v2 entries. CRC32 detects accidental corruption; it does not
authenticate untrusted data. The encoded-source filename key remains the existing
64-bit FNV-1a plus encoded length, so this change does not reduce key width.

The cache directory is now `Cache/decoded-images-v2`. Old v1 files are ignored and
rebuilt from source. Old folders are retained, not deleted automatically. The disk
budget applies to the active v2 directory; keeping v1 as well consumes additional
space. After closing ForgeSim, obsolete `decoded-images-v1` may be removed if its
cached data is no longer needed. No source assets or scene files are migrated.

Write checksums are computed from worker-owned immutable pixels before acquiring
the cache mutex. Only staging-file ownership, budget/eviction and file publication
remain serialized. Oversized entries still skip storage, failed reads rebuild,
and failures must preserve unrelated files. Read checksum validation remains
mandatory before a cache hit is accepted.

Tests exercise CRC32 known vectors, misaligned/tail lengths, portable/accelerated
agreement, valid-length pixel corruption, stale checksum rejection, busy leases,
eviction and failed writes. On the cannon's 64 MiB decoded normal image, Linux GCC
`-O2 -DNDEBUG` median checksum times after one warm-up were about 78.27 ms (FNV-1a)
and 2.55 ms (accelerated CRC32). This excludes decoding, allocation and disk I/O;
it is not an end-to-end speedup claim. Existing observations before this change
were 8.474 seconds with 58 misses and 3.425 seconds with 59 hits, in Release.
Repeat cache-population and cached-restart measurements using the new v2 directory.

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

## Keep image workers occupied

Parallel image loading now uses a per-model queue with at most four workers,
rather than waiting for an entire group of four before starting the next group.
A worker starts another image as soon as it finishes its previous image. Models
remain sequential; GPU work remains on the graphics-context thread. The serial
option is unchanged.

Results, progress consumption and decode errors are delivered in source-image
order, preserving image indices and deterministic error order. Futures join before
the source glTF document or worker-local timing records can be destroyed, including
on failure. Completed out-of-order results can wait in futures until earlier images
finish. This retains decoded images within the same final image payload budget;
there is no additional unbounded thread pool. Cache storage remains serialized
by its existing mutex and persistent format is unchanged.

A local Linux GCC `-O2 -DNDEBUG` check used a synthetic GLB with seven RGB PNGs:
one 4096-square image, three 512-square images, then three 4096-square images.
Images used Python Random seed 135 bytes and PNG compression level 1. Persistent
caching was disabled and four workers enabled. Across two alternating baseline/new
blocks, with each process's first iteration discarded, the six retained loads had
median times of 483.951 ms (batches) and 312.322 ms (queue). Measurements include
GLB file access, decode and a pixel checksum pass; OS caches were not cleared.
Decoded byte totals and pixel checksums matched. This is an uneven-work scheduling
probe, not a forecast of the Windows reference-scene speedup or a decoder change.

Repeat an empty-cache Release scene open with the same flags to measure the real
benefit against the 11.877-second observation below. An asset with at most four
images already starts them together, so the queue's main opportunity is larger
image sets with uneven decode times.

## Accelerated PNG decompression

`FORGESIM_ACCELERATE_PNG` defaults to ON. The build fetches libdeflate v1.25 at a
pinned commit and statically links its decompression support into Assets. Runtime
packages include its license and require no libdeflate DLL. Set the CMake option
to OFF for the original stb-only build; changing it requires a rebuild.

The integration replaces stb's whole-buffer PNG inflate operation. stb still
handles PNG parsing, filtering, palettes, transparency, channel expansion and
8/16-bit conversion. JPEG and HDR decoders are unchanged. Each inflate call owns
its decompressor, so image workers share no decompressor state. The raw DEFLATE
path used by CgBI and bounded growth beyond the PNG size estimate are retained.
The decoder's signed-int output-size contract remains the limit. libdeflate also
validates zlib checksums; malformed images can therefore fail earlier than with
stb's inflater. Persistent cache format and keys are unchanged.

Compatibility checks cover grayscale, RGB, alpha, transparency and Adam7 16-bit
PNGs; existing image/glTF tests cover RGBA8, HDR, cache hits/misses and corrupt cache
recovery. An isolated comparison on the uploaded cannon's 4096-square RGB16 normal
PNG checked all 67,108,864 RGBA8 bytes against the original decoder and found an
exact match. Linux GCC `-O2 -DNDEBUG` decode medians after discarding the first of
six iterations were 987.566 ms (stb) and 554.218 ms (libdeflate). Encoded bytes were
already in memory and GPU upload/cache work was excluded. The direct-channel
conversion candidate did not show a repeatable improvement and was not included.
This single-image result does not establish end-to-end Windows scene improvement.
Repeat the empty-cache Release scene measurement, followed by cached restart.

## Release reference-scene observations

Single user-captured Release runs after model reuse, before the worker-queue change:

| Scenario | Scene open |
| --- | ---: |
| Empty ForgeSim image cache | 11.877 s |
| Process restart, populated image cache | 3.602 s |
| Same-session reopen | 0.0469 s |

The restart had 59 cache hits and no misses. The reopen reused 17 model resources
and reloaded the external-dependency barrels asset. Hardware and controlled repeat
statistics remain to be recorded. These are distinct from the earlier Debug
measurements; do not treat the configuration change as a code optimization.

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
