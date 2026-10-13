#pragma once

#include <filesystem>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>

namespace ForgeSim::Assets
{
	[[nodiscard]] SceneAsset LoadGltfScene(
		const std::filesystem::path& path);

	// Convenience loader for assets containing exactly one
	// mesh with exactly one primitive.
	[[nodiscard]] MeshData LoadGltfMesh(
		const std::filesystem::path& path);
}