#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace ForgeSim::Assets
{
	struct MeshVertex
	{
		std::array<float, 3> position{};
		
		std::array<float, 3> normal{
			0.0f,
			0.0f,
			1.0f
		};

		std::array<float, 2> textureCoordinates{};

		std::array<float, 4> tangent{
			1.0f,
			0.0f,
			0.0f,
			1.0f
		};

		std::array<float, 3> color{
			1.0f,
			1.0f,
			1.0f
		};
	};

	struct MeshAttributeAvailability
	{
		bool normals = false;
		bool textureCoordinates = false;
		bool tangents = false;
		bool colors = false;
	};

	struct MeshData
	{
		std::vector<MeshVertex> vertices;
		std::vector<std::uint32_t> indices;

		MeshAttributeAvailability availableAttributes;
	};
}