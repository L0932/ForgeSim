# Decoded-image cache size investigation (#148)

The current v2 cache stores decoded RGBA8 pixels. This investigation compares
that raw payload with lossless raw DEFLATE at levels 1, 3 and 6 using the existing
pinned libdeflate dependency. The benchmark does not change the application cache
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
in libdeflate only when this benchmark is selected.

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
| `deflate_level` | 0 is raw storage; 1, 3, 6 are compression levels. |
| `stored_bytes`, `saved_percent` | Payload size and savings versus decoded bytes; excludes file headers. Negative savings mean expansion. |
| `compression_ms` | Median compression plus output-buffer preparation. Raw storage reports zero. |
| `write_io_ms` | Median payload file open/write/close; no durable flush. |
| `warm_read_restore_crc_ms` | Median read/allocation, decompression when applicable, and CRC32 validation of restored pixels. |
| `exact_pixels` | `yes` after full byte comparison for every run. |

Each candidate receives an unreported warm-up, then the requested measured runs.
Compressor/decompressor allocation is outside the timed loop; buffer resizing is
inside. Full comparison is outside the read timing, while CRC32 is inside.
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
The warm file reads used the OS cache. Windows measurements are still required.

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
existing v2 cache and its loading behavior remain unchanged by this increment.
