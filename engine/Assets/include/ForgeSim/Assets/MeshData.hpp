#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace ForgeSim::Assets
{
	struct MeshVertex
	{
		std::array<float, 3> position;
		std::array<float, 3> color;
	};

	struct MeshData
	{
		std::vector<MeshVertex> vertices;
		std::vector<std::uint32_t> indices;
	};
}