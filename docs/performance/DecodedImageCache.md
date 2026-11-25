# Decoded-image cache storage (#148)

Enable with `--cache-decoded-images` or Console `images.cache on`. Caching remains
optional. Eligible embedded GLB images now use `Cache/decoded-images-v3`; on Windows
this is normally `%LOCALAPPDATA%/ForgeSim/Cache/decoded-images-v3`.

## Selective storage

Images below 64 KiB of decoded RGBA8 data remain raw. Larger images are compressed
with LZ4 acceleration 1 only if they save at least 12.5% and at least 4 KiB of
pixel payload. Otherwise they remain raw. The 12.5% threshold is a prototype
heuristic, not a measured guarantee that every entry loads faster. Decoded pixels
and GPU texture formats are unchanged.

The 64-byte little-endian header records width, height, channels, checksum,
magic/version, codec, stored length and decoded length. Dimensions and both sizes
are checked against the file length and a 256 MiB decoded-image cap before
allocation. LZ4 reads must restore exactly the expected byte count; CRC32 then
checks restored pixels and dimensions. Corrupt, stale or unavailable entries fall
back to source decoding and are rebuilt when possible. Older raw headers are not
accepted as v3 entries.

Compression and checksum work run on immutable worker-owned pixels before the
cache mutex. Staging, budget eviction and publication remain serialized under
that mutex and the existing directory lease. Another importer bypasses a busy
cache rather than waiting for its lease. Each published entry is complete; pending
files left after termination are cleaned on a later scan under the lease.

The existing 4096 MiB default budget and `--image-cache-budget-mib` option count
**stored bytes including headers**, not decoded texture memory. During a write,
the staging file temporarily adds disk usage before publication/eviction.

## Memory and timing

Each compression attempt can allocate up to 87.5% of decoded pixel bytes in an
extra buffer. Compressed reads temporarily hold compressed and restored pixels.
Up to four workers may do this concurrently; the disk budget does not cap RAM or
VRAM. A failed compression allocation falls back to raw storage. Cache-read
allocation/I/O failures fall back to source decoding.

`--loading-diagnostics` adds `compress` and `decompress` to Image cache detail.
They are per-worker sums that may overlap. Compression is inside cache write work;
decompression is inside cache read work. Other checksum/I/O timings retain their
previous meanings. Use the scene-open completion time to judge total impact.

## Version migration and cleanup

The first v3 run populates a new cache even when v2 is populated. v1/v2 entries
are ignored and retained; their disk usage is outside the v3 budget. This allows
using an older build for comparison without destroying its cache.

To reclaim obsolete cache space, close all ForgeSim instances and benchmark tools,
open `%LOCALAPPDATA%/ForgeSim/Cache` in Explorer, and delete the obsolete
`decoded-images-v1` and/or `decoded-images-v2` folders. Keep v3 for warm-load
comparisons. Do not remove model or project asset folders. Automatic cleanup of
old versions is not implemented by this increment.

## Windows comparison before merging

1. Record Release scene-open timings with the previous v2 build using the same
   source assets, scene, loading flags and no debugger.
2. Build the v3 patch in Release. Run the same scene once to populate v3; capture
   scene-open duration, cache misses, bytes, compression time and peak process memory.
3. Close and relaunch, then reopen the scene and capture warm-cache results.
4. Reopen again in the same session to check model-resource reuse separately.
5. Repeat the sets and compare medians. Retain outliers and avoid calling a new
   ForgeSim cache a cold OS disk cache.
6. Review material appearance, displacement, node overrides, picking and shadows.
   Run the full Windows CTest suite and package smoke test, including the LZ4 notice.

The candidate cannon measurements suggest about 31% payload savings with an
approximately 12% warm restore overhead in the isolated Windows benchmark. These
are not whole-scene or integrated v3 results. The prototype needs the comparisons
above before deciding whether to enable it for a release. #148 remains open.

## Local integration verification

Release/O2 headless GLB imports verified every image byte against uncached source
loading. The cold ForgeSim-cache run produced these v3 entries; each following
load hit every image. Reads were OS-cached and these were individual-model CPU
imports, not native Windows scene-open/GPU timings.

| Model | LZ4 entries | Raw entries | Decoded MiB | Stored MiB incl. headers | Savings vs. pixels |
| --- | ---: | ---: | ---: | ---: | ---: |
| cannon | 3 | 0 | 192.00 | 132.87 | 30.8% |
| medieval | 1 | 2 | 12.00 | 9.29 | 22.6% |
| planks | 4 | 1 | 16.00 | 6.89 | 57.0% |

Codec, GLB/cache, image loading, 16-bit PNG and mesh-displacement tests passed with
PNG acceleration enabled and disabled. Coverage includes raw fallback, exact alpha
values, malformed dimensions/lengths/codecs, compressed corruption/truncation,
stored-byte budgets, eviction, busy-cache fallback and concurrent imports.
Windows full-suite, packaging and end-to-end performance validation remain pending.
