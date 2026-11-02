#pragma once

#include <filesystem>
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

	struct GltfImportOptions
	{
		GltfTexturePolicy texturePolicy = GltfTexturePolicy::Strict;

		// Called synchronously for each skipped primitive texture usage.
		// The message is valid only for the duration of the callback.
		std::function<void(std::string_view)> onWarning;
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
