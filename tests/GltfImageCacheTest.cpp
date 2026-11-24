#include <ForgeSim/Assets/GltfLoader.hpp>

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "../engine/Assets/src/DecodedImageCacheBudget.hpp"

namespace
{
	namespace fs = std::filesystem;
	using namespace ForgeSim::Assets;

	struct TestDirectory
	{
		fs::path path = fs::temp_directory_path() /
			("ForgeSim-cache-test-" + std::to_string(std::random_device{}()));
		TestDirectory()
		{
			if (!fs::create_directory(path))
				throw std::runtime_error("Could not create cache test directory.");
		}
		~TestDirectory()
		{
			std::error_code ignored;
			fs::remove_all(path, ignored);
		}
	};

	void Require(bool condition, const char* message)
	{
		if (!condition)
			throw std::runtime_error(message);
	}

	void WriteGlb(const fs::path& path, unsigned char key)
	{
		// A one-pixel PNG. The final padding byte is part of the image buffer
		// view, so each fixture has a distinct encoded cache key.
		const std::array<unsigned char, 72> png{
			137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82,
			0, 0, 0, 1, 0, 0, 0, 1, 8, 2, 0, 0, 0, 144, 119, 83,
			222, 0, 0, 0, 12, 73, 68, 65, 84, 120, 156, 99, 248, 207, 192, 0,
			0, 3, 1, 1, 0, 201, 254, 146, 239, 0, 0, 0, 0, 73, 69, 78,
			68, 174, 66, 96, 130, 0, 0, key,
		};
		std::string json = R"({"asset":{"version":"2.0"},"buffers":[{"byteLength":72}],)"
			R"("bufferViews":[{"buffer":0,"byteLength":72}],)"
			R"("images":[{"bufferView":0,"mimeType":"image/png"}]})";
		json.append((4 - json.size() % 4) % 4, ' ');
		std::ofstream out(path, std::ios::binary);
		const auto word = [&](std::uint32_t value)
		{
			const std::array<unsigned char, 4> bytes{
				static_cast<unsigned char>(value), static_cast<unsigned char>(value >> 8),
				static_cast<unsigned char>(value >> 16), static_cast<unsigned char>(value >> 24)
			};
			out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		};
		out.write("glTF", 4);
		word(2);
		word(static_cast<std::uint32_t>(28 + json.size() + png.size()));
		word(static_cast<std::uint32_t>(json.size()));
		out.write("JSON", 4);
		out.write(json.data(), static_cast<std::streamsize>(json.size()));
		word(static_cast<std::uint32_t>(png.size()));
		out.write("BIN\0", 4);
		out.write(reinterpret_cast<const char*>(png.data()), png.size());
		Require(static_cast<bool>(out), "Could not write cache test GLB.");
	}
}

void RunGltfImageCacheTests()
{
	TestDirectory directory;
	const auto a = directory.path / "a.glb";
	const auto b = directory.path / "b.glb";
	const auto c = directory.path / "c.glb";
	WriteGlb(a, 1);
	WriteGlb(b, 2);
	WriteGlb(c, 3);

	GltfImportOptions options;
	options.imageCacheDirectory = directory.path / "Cache";
	const auto cache = options.imageCacheDirectory / DecodedImageCacheDirectoryName;
	const auto load = [&](const fs::path& asset)
	{
		std::optional<GltfImageCacheStats> stats;
        std::optional<GltfImageLoadTimings> timing;
		options.onProgress = [&](const GltfLoadProgress& progress)
		{
			if (progress.imageCache)
				stats = progress.imageCache;
            if (progress.imageTimings) timing = progress.imageTimings;
		};
		const auto scene = LoadGltfScene(asset, options);
		options.onProgress = {};
		Require(scene.textureResources.images.at(0).pixels ==
			std::vector<std::uint8_t>{255, 0, 0, 255}, "Cache changed decoded pixels.");
        Require(timing.has_value(), "Image completion must report timing even without caching.");
        Require(timing->decodedImages == (stats && stats->available ? stats->misses : 1),
            "Warm hits must skip decoding; misses and bypasses must decode.");
        Require(timing->cacheSetupMs >= 0 && timing->cacheKeyMs >= 0 && timing->cacheReadMs >= 0 &&
            timing->decodeMs >= 0 && timing->cacheWriteMs >= 0, "Image times must be nonnegative.");
        Require(timing->cacheLockWaitMs >= 0 && timing->cacheReadIoMs >= 0 &&
            timing->cacheReadChecksumMs >= 0 && timing->cacheWriteChecksumMs >= 0 &&
            timing->cacheWriteIoMs >= 0 && timing->cachePublishMs >= 0, "Cache detail times must be nonnegative.");
        if (timing->decodedImages == 0)
            Require(timing->cacheWriteChecksumMs == 0 && timing->cacheWriteIoMs == 0 && timing->cachePublishMs == 0,
                "Warm hits must not report write operations.");
        if (!stats || !stats->available)
            Require(timing->cacheReadIoMs == 0 && timing->cacheReadChecksumMs == 0 &&
                timing->cacheWriteIoMs == 0 && timing->cacheWriteChecksumMs == 0,
                "Bypassed cache must not report pixel I/O/checksums.");
        Require(timing->workerLimit >= 1 && timing->workerLimit <= 4, "Bounded worker count.");
		return stats;
	};

	Require(load(a)->misses == 1, "A should miss.");
	const auto entryA = [&]
	{
		for (const auto& entry : fs::directory_iterator(cache))
			if (entry.path().filename() != ".budget.lock")
				return entry.path();
		throw std::runtime_error("Missing cache entry A.");
	}();
	const auto bytes = fs::file_size(entryA);
    {
        std::fstream entry(entryA, std::ios::in | std::ios::out | std::ios::binary);
        entry.seekp(4 * sizeof(std::uint64_t));
        entry.put('\0'); // First cached byte should be red=255.
    }
    Require(load(a)->misses == 1 && load(a)->hits == 1, "Pixel corruption must rebuild then hit.");
    // A v1 checksum placed in the v2 directory must not be trusted.
    const auto legacy = directory.path / "Cache" / "decoded-images-v1";
    fs::create_directory(legacy);
    std::ofstream(legacy / "keep-old-cache") << "retained";
    Require(load(a)->hits == 1 && fs::exists(legacy / "keep-old-cache"), "Legacy directory must remain untouched.");
    {
        std::uint64_t fnv = 14695981039346656037ull;
        for (auto byte : std::vector<std::uint8_t>{255,0,0,255}) fnv = (fnv ^ byte) * 1099511628211ull;
        const std::uint64_t oldChecksum = fnv ^ (std::uint64_t{1} << 32) ^ 1;
        std::fstream entry(entryA, std::ios::in | std::ios::out | std::ios::binary);
        entry.seekp(3 * sizeof(std::uint64_t));
        entry.write(reinterpret_cast<const char*>(&oldChecksum),sizeof(oldChecksum));
    }
    Require(load(a)->misses == 1 && load(a)->hits == 1, "Legacy checksum must rebuild safely.");

	options.imageCacheBudgetBytes = 2 * bytes;
	Require(load(b)->bytesAfter == 2 * bytes, "A and B should both fit.");
	fs::path entryB;
	for (const auto& entry : fs::directory_iterator(cache))
		if (entry.path() != entryA && entry.path().filename() != ".budget.lock")
			entryB = entry.path();
	const auto old = fs::file_time_type::clock::now() - std::chrono::hours{24};
	fs::last_write_time(entryA, old - std::chrono::hours{1});
	fs::last_write_time(entryB, old);
	Require(load(a)->hits == 1, "A should hit and become recent.");
	const auto inserted = load(c).value();
	Require(inserted.evictions == 1 && inserted.bytesAfter == 2 * bytes &&
		!fs::exists(entryB) && load(a)->hits == 1, "C should evict B, retaining A.");

	const auto unrelated = cache / "notes.txt";
	std::ofstream(unrelated) << "keep";
	options.imageCacheBudgetBytes = bytes;
	const auto lowered = load(a).value();
	Require(lowered.hits == 1 && lowered.bytesBefore == 2 * bytes &&
		lowered.bytesAfter == bytes && lowered.evictions == 1, "Lowered budget should trim on a hit.");

	// Hold the same OS lease as another loader to exercise the busy fallback.
	{
		Detail::DecodedImageCacheLease held(cache / ".budget.lock");
		Require(held.Acquired() && !load(a)->available, "Busy cache should bypass without waiting.");
	}

	std::ofstream(entryA, std::ios::trunc) << "bad";
	auto blockedWrite = entryA;
	blockedWrite += ".pending";
	fs::create_directory(blockedWrite);
	std::ofstream(blockedWrite / "keep") << "keep";
	const auto failed = load(a).value();
	Require(failed.misses == 1 && failed.skippedWrites == 1 && fs::exists(blockedWrite / "keep"),
		"Failed cache write should decode normally and preserve unrelated files.");
	fs::remove_all(blockedWrite);
	Require(load(a)->misses == 1 && load(a)->hits == 1, "Corrupt cache entry should be rebuilt.");
	options.imageCacheBudgetBytes = bytes - 1;
	const auto oversized = load(a).value();
	Require(oversized.skippedWrites == 1 && oversized.bytesAfter == 0 && fs::exists(unrelated),
		"Oversized image should bypass storage and unrelated files should survive.");

	options.imageCacheDirectory.clear();
	Require(!load(a), "Disabled cache should not report cache work.");
    options.imageCacheDirectory = directory.path / "ParallelCache";
    options.parallelImageDecode = true;
    options.imageCacheBudgetBytes = 2 * bytes;
    Require(load(a)->misses == 1 && load(a)->hits == 1, "Parallel option must retain timing and cache behavior.");
}
