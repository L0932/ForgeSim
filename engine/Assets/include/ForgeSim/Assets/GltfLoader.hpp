#pragma once

#include <filesystem>

#include <ForgeSim/Assets/MeshData.hpp>

namespace ForgeSim::Assets
{
	[[nodiscard]] MeshData LoadGltfMesh(
		const std::filesystem::path& path
	);
}