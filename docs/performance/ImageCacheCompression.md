# Decoded-image cache size investigation (#148)

Runtime follow-up: the prototype now uses selective LZ4 acceleration 1 in the v3
cache. See [decoded-image cache storage](DecodedImageCache.md). The benchmark
results and earlier investigation notes below describe the candidate experiments;
the comparison tool itself still does not modify the application cache.


The current v2 cache stores decoded RGBA8 pixels. This investigation compares
that raw payload with lossless raw DEFLATE at levels 1, 3 and 6, and LZ4 block
compression at acceleration 1 and 4. libdeflate uses the existing pinned dependency;
LZ4 v1.10.0 is pinned as a shared static dependency. The benchmark does not change the application cache
format, migrate entries, or delete cache directories.

## Build and run on Windows

Use a Release build and run outside the debugger:

```bash
cmake -S . -B out/build/x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release -DFORGESIM_BUILD_CACHE_BENCHMARK=ON
cmake --build out/build/x64-Release --target ForgeSimCacheBenchmark --parallel
./out/build/x64-Release/tools/cache-benchmark/ForgeSimCacheBenchmark.exe "C:/MyAssets/model.glb" 3 > cache-comparison.csv
```

The tool requires `FORGESIM_ACCELERATE_PNG=ON` (the default). It is excluded from
runtime packages and normal builds. Disable it afterward with
`-DFORGESIM_BUILD_CACHE_BENCHMARK=OFF` if desired. Compression support is enabled
in libdeflate only when this benchmark is selected. Initial configuration also
fetches the pinned LZ4 source. The runtime now shares its static dependency; no LZ4 DLL is required.
The benchmark's LZ4 license is copied beside its executable.

Pass a GLB, glTF, or an individual image. The optional run count must be odd,
between 3 and 21; default 3. GLB/glTF files are loaded without persistent image
caching. Each decoded image is tested separately, serially. Every measured and
warm-up run compares all restored pixel bytes against the decoded source; any
mismatch or I/O failure exits unsuccessfully. No source assets are modified.

Output is CSV on stdout, with source-load context/errors on stderr. Check the
exit code before treating an output file as a complete successful run. Scratch
payloads go in a unique system temporary directory and are removed on normal
completion or a caught error. A terminated process may leave its own
`ForgeSim-cache-benchmark-*` temporary directory behind.

## Interpret the columns

| Column | Meaning |
| --- | --- |
| `image`, `width`, `height` | Source image index and decoded dimensions. |
| `decoded_bytes` | Original RGBA8 pixel payload. |
| `codec` | `raw`, `deflate`, or `lz4`. |
| `parameter` | 0 for raw; compression level 1/3/6 for DEFLATE; acceleration 1/4 for LZ4. |
| `stored_bytes`, `saved_percent` | Payload size and savings versus decoded bytes; excludes file headers. Negative savings mean expansion. |
| `compression_ms` | Median compression plus output-buffer preparation. Raw storage reports zero. |
| `write_io_ms` | Median payload file open/write/close; no durable flush. |
| `warm_read_restore_crc_ms` | Median read/allocation, decompression when applicable, and CRC32 validation of restored pixels. |
| `exact_pixels` | `yes` after full byte comparison for every run. |

Each candidate receives an unreported warm-up, then the requested measured runs.
DEFLATE compressor/decompressor allocation is outside the timed loop; buffer
resizing is inside. LZ4 uses its stateless compression/decompression entry points. Full comparison is outside the read timing, while CRC32 is inside.
Medians are computed independently and need not add up to a median total.

These reads follow writes to the same scratch file and therefore measure an
**OS-cached microbenchmark**, not cold disk access or application restart loading.
Writing is buffered by the OS, too. CPU, GPU and full-scene memory peaks are not
measured. This benchmark retains all source decoded images and temporary buffers;
its memory behavior is not the proposed runtime cache behavior.

## Decision and integration follow-up

Use representative normal, color, packed material and alpha images at several
resolutions. Compare savings against both population cost and warm restore cost.
Prefer selective storage if poorly compressible images expand or restore slowly.
Pixel equivalence is necessary; the final integration also requires rendering
checks and whole-scene Release timings following the existing scene-loading guide.

A runtime-format change will need explicit header/version validation, bounded
sizes, corruption recovery, budgets based on stored bytes, safe concurrent
publication, and a cleanup path for obsolete versions. Those remain separate
implementation steps after choosing the format; this benchmark does not mark
#148 complete.

## Initial local measurements

Release/O2 build, three measured repetitions after a warm-up, serial images.
The supplied source exports are local reference assets, not added to the repository.
Times below sum independent per-image medians; they are not scene-open durations.
The warm file reads used the OS cache. These initial rows predate the Windows
measurements below.

| Asset | Images | Raw MiB | DEFLATE level 1 MiB | Savings | Raw restore ms | Compressed restore ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| cannon | 3 | 192.0 | 91.0 | 52.6% | 99.0 | 415.5 |
| medieval | 3 | 12.0 | 6.3 | 47.2% | 1.4 | 24.1 |
| planks | 5 | 16.0 | 4.2 | 73.4% | 1.7 | 17.5 |

All candidates at levels 1, 3 and 6 restored every decoded byte exactly. For the
three 4K cannon images, level 1 took approximately 262–462 ms per image to
compress, in addition to writing. Higher levels generally improved size only
modestly while increasing compression cost.

On these runs, plain DEFLATE reduces storage but substantially increases warm
restore cost. It is not selected as a runtime default. Investigate faster restore
strategies and selective storage before changing the production format. The
v2 cache remained unchanged during that benchmark increment; the later runtime
follow-up is described at the top of this guide.

## Windows DEFLATE comparison

The supplied Windows CSV contains three 4096x4096 decoded images matching the
cannon reference payload sizes. All restored pixels matched. Times are sums of
independent per-image medians, not full-scene durations or cold disk timings.

| Codec/level | Payload MiB | Savings | Compression ms | Write ms | Warm restore+CRC ms |
| --- | ---: | ---: | ---: | ---: | ---: |
| Raw | 192.00 | 0% | 0 | 164.64 | 163.33 |
| DEFLATE 1 | 90.98 | 52.61% | 777.88 | 81.04 | 368.92 |
| DEFLATE 3 | 88.72 | 53.79% | 1330.64 | 118.97 | 351.87 |
| DEFLATE 6 | 86.22 | 55.09% | 2080.66 | 78.77 | 347.30 |

This supports evaluating LZ4 before selecting a runtime representation. After
applying this increment, rerun the command above and use a new output filename
such as `cache-comparison-lz4.csv`. CSV now uses `codec` and `parameter` instead
of the earlier `deflate_level` column. Keep the existing runtime cache; this tool
continues to use separate scratch files.

## Local LZ4 candidate measurements

Release/O2, three measured repetitions after warm-up, same source exports as
above. All six candidates passed exact pixel comparison on each image. Tiny,
nonopaque-alpha and incompressible image fixtures also passed; LZ4 expands the
small/noisy inputs in these fixtures, so selective raw fallback remains relevant.
These are OS-cached, serial microbenchmarks, not Windows or whole-scene results.

| Asset | Candidate | Stored MiB | Savings | Compression ms | Warm restore+CRC ms |
| --- | --- | ---: | ---: | ---: | ---: |
| cannon | raw 0 | 192.00 | 0.0% | 0.00 | 105.86 |
| cannon | deflate 1 | 90.98 | 52.6% | 1112.89 | 415.81 |
| cannon | lz4 1 | 132.87 | 30.8% | 386.26 | 189.56 |
| cannon | lz4 4 | 138.09 | 28.1% | 294.10 | 184.39 |
| medieval | raw 0 | 12.00 | 0.0% | 0.00 | 1.46 |
| medieval | deflate 1 | 6.33 | 47.2% | 82.55 | 25.81 |
| medieval | lz4 1 | 9.03 | 24.7% | 13.68 | 4.18 |
| medieval | lz4 4 | 9.21 | 23.3% | 8.87 | 4.24 |
| planks | raw 0 | 16.00 | 0.0% | 0.00 | 1.54 |
| planks | deflate 1 | 4.25 | 73.4% | 71.51 | 19.10 |
| planks | lz4 1 | 6.89 | 57.0% | 23.78 | 5.55 |
| planks | lz4 4 | 7.37 | 54.0% | 18.56 | 5.40 |

LZ4 reduces restore overhead relative to DEFLATE on these reference textures,
but raw storage remains faster to restore here. Do not select a production codec
from storage savings alone; compare the updated tool on Windows next. Candidate
order is fixed, the OS cache is warm, and these results do not isolate storage
hardware throughput. At the benchmark stage the v2 runtime cache remained unchanged; the later runtime
follow-up uses the versioned v3 cache described above.
