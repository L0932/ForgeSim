#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ForgeSim::Assets
{
	enum class AlphaMode : std::uint8_t
	{
		Opaque,
		Mask,
		Blend
	};

	struct MaterialTextureReference
	{
		std::size_t textureIndex;
		std::uint32_t textureCoordinateSet = 0;
	};

	struct NormalTextureReference
	{
		MaterialTextureReference texture;
		float scale = 1.0f;
	};

	struct OcclusionTextureReference
	{
		MaterialTextureReference texture;
		float strength = 1.0f;
	};

	struct MetallicRoughnessData
	{
		std::array<float, 4> baseColorFactor{
			1.0f,
			1.0f,
			1.0f,
			1.0f
		};

		std::optional<MaterialTextureReference>
			baseColorTexture;

		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;

		std::optional<MaterialTextureReference>
			metallicRoughnessTexture;
	};

	struct MaterialData
	{
		MetallicRoughnessData metallicRoughness;

		std::optional<NormalTextureReference>
			normalTexture;

		std::optional<OcclusionTextureReference>
			occlusionTexture;

		std::array<float, 3> emissiveFactor{
			0.0f,
			0.0f,
			0.0f
		};

		std::optional<MaterialTextureReference>
			emissiveTexture;

		AlphaMode alphaMode = AlphaMode::Opaque;
		float alphaCutoff = 0.5f;
		bool doubleSided = false;
	};
}