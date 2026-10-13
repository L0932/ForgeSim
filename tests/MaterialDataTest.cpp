#include <ForgeSim/Assets/MaterialData.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace
{
	[[nodiscard]] bool NearlyEqual(
		float left,
		float right,
		float tolerance = 1e-6f) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}
}

int main()
{
	using ForgeSim::Assets::AlphaMode;
	using ForgeSim::Assets::MaterialData;
	using ForgeSim::Assets::MaterialTextureReference;
	using ForgeSim::Assets::NormalTextureReference;
	using ForgeSim::Assets::OcclusionTextureReference;

	static_assert(
		std::is_copy_constructible_v<MaterialData>);

	static_assert(
		std::is_copy_assignable_v<MaterialData>);

	static_assert(
		std::is_move_constructible_v<MaterialData>);

	static_assert(
		std::is_move_assignable_v<MaterialData>);

	// Default construction should use glTF-compatible defaults.
	{
		const MaterialData material;

		const auto& metallicRoughness =
			material.metallicRoughness;

		for (const float component :
		metallicRoughness.baseColorFactor)
		{
			if (!NearlyEqual(component, 1.0f))
			{
				std::cerr
					<< "Expected a white base-color factor\n";

				return EXIT_FAILURE;
			}
		}

		if (metallicRoughness.baseColorTexture)
		{
			std::cerr
				<< "Expected no default base-color texture\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			metallicRoughness.metallicFactor,
			1.0f))
		{
			std::cerr
				<< "Expected the default metallic factor "
				<< "to be one\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			metallicRoughness.roughnessFactor,
			1.0f))
		{
			std::cerr
				<< "Expected the default roughness factor "
				<< "to be one\n";

			return EXIT_FAILURE;
		}

		if (metallicRoughness.metallicRoughnessTexture ||
			material.normalTexture ||
			material.occlusionTexture ||
			material.emissiveTexture)
		{
			std::cerr
				<< "Expected no default material textures\n";

			return EXIT_FAILURE;
		}

		for (const float component :
		material.emissiveFactor)
		{
			if (!NearlyEqual(component, 0.0f))
			{
				std::cerr
					<< "Expected a black emissive factor\n";

				return EXIT_FAILURE;
			}
		}

		if (material.alphaMode != AlphaMode::Opaque)
		{
			std::cerr
				<< "Expected opaque alpha mode by default\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(material.alphaCutoff, 0.5f))
		{
			std::cerr
				<< "Expected an alpha cutoff of 0.5\n";

			return EXIT_FAILURE;
		}

		if (material.doubleSided)
		{
			std::cerr
				<< "Expected single-sided rendering by default\n";

			return EXIT_FAILURE;
		}

		const MaterialTextureReference textureReference{
			.textureIndex = 0
		};

		if (textureReference.textureCoordinateSet != 0)
		{
			std::cerr
				<< "Expected texture-coordinate set zero by default\n";

			return EXIT_FAILURE;
		}
	}

	// Explicit material values should be retained.
	{
		MaterialData material;

		material.metallicRoughness.baseColorFactor = {
			0.25f,
			0.5f,
			0.75f,
			0.8f
		};

		material.metallicRoughness.baseColorTexture =
			MaterialTextureReference{
				.textureIndex = 2,
				.textureCoordinateSet = 1
		};

		material.metallicRoughness.metallicFactor =
			0.2f;

		material.metallicRoughness.roughnessFactor =
			0.7f;

		material.metallicRoughness
			.metallicRoughnessTexture =
			MaterialTextureReference{
				.textureIndex = 3,
				.textureCoordinateSet = 0
		};

		material.normalTexture =
			NormalTextureReference{
				.texture = {
					.textureIndex = 4,
					.textureCoordinateSet = 1
				},
				.scale = 0.6f
		};

		material.occlusionTexture =
			OcclusionTextureReference{
				.texture = {
					.textureIndex = 5,
					.textureCoordinateSet = 0
				},
				.strength = 0.4f
		};

		material.emissiveFactor = {
			0.1f,
			0.2f,
			0.3f
		};

		material.emissiveTexture =
			MaterialTextureReference{
				.textureIndex = 6,
				.textureCoordinateSet = 1
		};

		material.alphaMode = AlphaMode::Blend;
		material.alphaCutoff = 0.25f;
		material.doubleSided = true;

		const MaterialData copiedMaterial = material;
		MaterialData movedMaterial =
			std::move(material);

		const auto& metallicRoughness =
			movedMaterial.metallicRoughness;

		if (!NearlyEqual(
			metallicRoughness.baseColorFactor[0],
			0.25f) ||
			!NearlyEqual(
				metallicRoughness.baseColorFactor[1],
				0.5f) ||
			!NearlyEqual(
				metallicRoughness.baseColorFactor[2],
				0.75f) ||
			!NearlyEqual(
				metallicRoughness.baseColorFactor[3],
				0.8f))
		{
			std::cerr
				<< "Base-color factor was not retained\n";

			return EXIT_FAILURE;
		}

		if (!metallicRoughness.baseColorTexture ||
			metallicRoughness.baseColorTexture
			->textureIndex != 2 ||
			metallicRoughness.baseColorTexture
			->textureCoordinateSet != 1)
		{
			std::cerr
				<< "Base-color texture reference was not retained\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			metallicRoughness.metallicFactor,
			0.2f) ||
			!NearlyEqual(
				metallicRoughness.roughnessFactor,
				0.7f))
		{
			std::cerr
				<< "Metallic-roughness factors were not retained\n";

			return EXIT_FAILURE;
		}

		if (!metallicRoughness.metallicRoughnessTexture ||
			metallicRoughness.metallicRoughnessTexture
			->textureIndex != 3 ||
			metallicRoughness.metallicRoughnessTexture
			->textureCoordinateSet != 0)
		{
			std::cerr
				<< "Metallic-roughness texture reference "
				<< "was not retained\n";

			return EXIT_FAILURE;
		}

		if (!movedMaterial.normalTexture ||
			movedMaterial.normalTexture
			->texture.textureIndex != 4 ||
			movedMaterial.normalTexture
			->texture.textureCoordinateSet != 1 ||
			!NearlyEqual(
				movedMaterial.normalTexture->scale,
				0.6f))
		{
			std::cerr
				<< "Normal texture data was not retained\n";

			return EXIT_FAILURE;
		}

		if (!movedMaterial.occlusionTexture ||
			movedMaterial.occlusionTexture
			->texture.textureIndex != 5 ||
			movedMaterial.occlusionTexture
			->texture.textureCoordinateSet != 0 ||
			!NearlyEqual(
				movedMaterial.occlusionTexture->strength,
				0.4f))
		{
			std::cerr
				<< "Occlusion texture data was not retained\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			movedMaterial.emissiveFactor[0],
			0.1f) ||
			!NearlyEqual(
				movedMaterial.emissiveFactor[1],
				0.2f) ||
			!NearlyEqual(
				movedMaterial.emissiveFactor[2],
				0.3f))
		{
			std::cerr
				<< "Emissive factor was not retained\n";

			return EXIT_FAILURE;
		}

		if (!movedMaterial.emissiveTexture ||
			movedMaterial.emissiveTexture
			->textureIndex != 6 ||
			movedMaterial.emissiveTexture
			->textureCoordinateSet != 1)
		{
			std::cerr
				<< "Emissive texture reference was not retained\n";

			return EXIT_FAILURE;
		}

		if (movedMaterial.alphaMode != AlphaMode::Blend ||
			!NearlyEqual(
				movedMaterial.alphaCutoff,
				0.25f) ||
			!movedMaterial.doubleSided)
		{
			std::cerr
				<< "Rendering properties were not retained\n";

			return EXIT_FAILURE;
		}

		if (!copiedMaterial
			.metallicRoughness
			.baseColorTexture ||
			copiedMaterial
			.metallicRoughness
			.baseColorTexture
			->textureIndex != 2)
		{
			std::cerr
				<< "Copied material did not retain its data\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}