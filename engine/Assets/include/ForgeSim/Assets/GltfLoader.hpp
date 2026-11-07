#pragma once

#include <chrono>
#include <cstddef>
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

	struct GltfLoadProgress
	{
		std::string_view stage;
		GltfProgressPhase phase;
		std::size_t completed;
		std::optional<std::size_t> total;
		std::chrono::milliseconds elapsed;
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
