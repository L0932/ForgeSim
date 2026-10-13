#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <ForgeSim/Assets/MaterialData.hpp>
#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/TextureData.hpp>

namespace ForgeSim::Assets
{
	using SceneNodeIndex = std::size_t;
	using SceneMeshIndex = std::size_t;
	using MaterialIndex = std::size_t;

	struct SceneNodeTransform
	{
		std::array<float, 3> translation{
			0.0f,
			0.0f,
			0.0f
		};

		// glTF quaternion component order: X, Y, Z, W.
		std::array<float, 4> rotation{
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};

		std::array<float, 3> scale{
			1.0f,
			1.0f,
			1.0f
		};

		// When present, the matrix is authoritative and the
		// translation, rotation, and scale values are ignored.
		// glTF matrices use column-major component order.
		std::optional<std::array<float, 16>> matrix;
	};

	struct MeshPrimitiveData
	{
		MeshData geometry;
		std::optional<MaterialIndex> materialIndex;
	};

	struct SceneMeshData
	{
		std::string name;
		std::vector<MeshPrimitiveData> primitives;
	};

	struct SceneNodeData
	{
		std::string name;
		SceneNodeTransform localTransform;

		std::optional<SceneMeshIndex> meshIndex;
		std::vector<SceneNodeIndex> childIndices;
	};

	struct SceneAsset
	{
		std::string name;

		std::vector<SceneNodeIndex> rootNodeIndices;
		std::vector<SceneNodeData> nodes;
		std::vector<SceneMeshData> meshes;

		std::vector<MaterialData> materials;
		TextureResources textureResources;
	};
}