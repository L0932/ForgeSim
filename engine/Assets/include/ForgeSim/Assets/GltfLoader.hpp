#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <functional>
#include <string_view>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>

namespace ForgeSim::Assets
{
	enum class GltfTexturePolicy
	{
		Strict,
		SkipUnusableTextures
	};

	enum class GltfProgressPhase { Started, Advanced, Completed };

	inline constexpr std::uint64_t ImageCacheMiB = 1024ull * 1024;
	inline constexpr std::uint64_t DefaultImageCacheBudgetMiB = 4096;

	struct GltfImageCacheStats
	{
		std::uint64_t budgetBytes = 0;
		std::uint64_t bytesBefore = 0;
		std::uint64_t bytesAfter = 0;
		std::size_t hits = 0;
		std::size_t misses = 0;
		std::size_t evictions = 0;
		std::size_t skippedWrites = 0;
		bool available = false;
	};

	struct GltfLoadProgress
	{
		std::string_view stage;
		GltfProgressPhase phase;
		std::size_t completed;
		std::optional<std::size_t> total;
		std::chrono::milliseconds elapsed;
		std::optional<GltfImageCacheStats> imageCache = std::nullopt;
	};

	struct GltfImportOptions
	{
		GltfTexturePolicy texturePolicy = GltfTexturePolicy::Strict;

		// Called synchronously for each skipped primitive texture usage.
		// The message is valid only for the duration of the callback.
		std::function<void(std::string_view)> onWarning;

		// Optional synchronous observer. A stage starts before any of its work.
		std::function<void(const GltfLoadProgress&)> onProgress;

		// Decode independent images concurrently while the scene load remains synchronous.
		bool parallelImageDecode = false;

		// Optional persistent cache for decoded images embedded in GLBs.
		std::filesystem::path imageCacheDirectory;
		std::uint64_t imageCacheBudgetBytes = DefaultImageCacheBudgetMiB * ImageCacheMiB;
	};

	// Strict preserves the existing import behavior. The opt-in fallback
	// retains material factors and skips texture usages that cannot use
	// the primitive's primary UVs. It neither repairs nor generates UVs.
	[[nodiscard]] SceneAsset LoadGltfScene(
		const std::filesystem::path& path,
		const GltfImportOptions& options = {});

	// Convenience loader for assets containing exactly one
	// mesh with exactly one primitive.
	[[nodiscard]] MeshData LoadGltfMesh(
		const std::filesystem::path& path);
}
