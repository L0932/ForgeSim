#include <ForgeSim/Assets/GltfLoader.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	void Expect(bool condition, const char* message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	bool HasAllTextures(const ForgeSim::Assets::MaterialData& material)
	{
		return material.metallicRoughness.baseColorTexture &&
			material.metallicRoughness.metallicRoughnessTexture &&
			material.normalTexture && material.occlusionTexture &&
			material.emissiveTexture;
	}

	bool HasNoTextures(const ForgeSim::Assets::MaterialData& material)
	{
		return !material.metallicRoughness.baseColorTexture &&
			!material.metallicRoughness.metallicRoughnessTexture &&
			!material.normalTexture && !material.occlusionTexture &&
			!material.emissiveTexture;
	}
}

void RunGltfTextureFallbackTests()
{
	using namespace ForgeSim::Assets;
	const std::filesystem::path directory{ FORGESIM_TEST_ASSET_DIRECTORY };
	const auto sharedPath = directory / "SharedMaterialMissingUvs.gltf";
	std::vector<std::string> warnings;
	const GltfImportOptions fallback{
		.texturePolicy = GltfTexturePolicy::SkipUnusableTextures,
		.onWarning = [&warnings](std::string_view message)
		{
			warnings.emplace_back(message);
		}
	};

	// Existing strict behavior is retained, including the loader's handling
	// of absent UVs. The renderer remains responsible for its strict checks.
	const auto strict = LoadGltfScene(sharedPath);
	Expect(strict.materials.size() == 1 && HasAllTextures(strict.materials[0]),
		"Default import changed shared source material textures.");

	const auto scene = LoadGltfScene(sharedPath, fallback);
	const auto& primitives = scene.meshes.at(0).primitives;
	Expect(primitives.size() == 3 && scene.materials.size() == 2,
		"Expected one textured and one shared fallback material variant.");
	const auto& textured = scene.materials.at(primitives[0].materialIndex.value());
	const auto& untextured = scene.materials.at(primitives[1].materialIndex.value());
	Expect(HasAllTextures(textured) && HasNoTextures(untextured),
		"Fallback modified valid shared material textures or kept unusable textures.");
	Expect(primitives[1].materialIndex == primitives[2].materialIndex,
		"Equivalent fallback variants were duplicated.");
	Expect(primitives[0].geometry.availableAttributes.textureCoordinates &&
		!primitives[1].geometry.availableAttributes.textureCoordinates,
		"Fallback generated or removed UV geometry.");
	Expect(untextured.metallicRoughness.baseColorFactor ==
		textured.metallicRoughness.baseColorFactor &&
		untextured.metallicRoughness.metallicFactor == 0.25f &&
		untextured.metallicRoughness.roughnessFactor == 0.75f &&
		untextured.emissiveFactor == textured.emissiveFactor &&
		untextured.alphaMode == AlphaMode::Mask &&
		untextured.alphaCutoff == 0.4f && untextured.doubleSided,
		"Fallback changed authored material factors or surface properties.");
	Expect(warnings.size() == 10,
		"Expected one warning per skipped primitive texture usage.");
	Expect(warnings[0].find(sharedPath.string()) != std::string::npos &&
		warnings[0].find("mesh 0, primitive 1, material 0 ('Shared')") != std::string::npos &&
		warnings[0].find("base color") != std::string::npos &&
		warnings[0].find("TEXCOORD_0") != std::string::npos,
		"Fallback warning omitted asset, primitive, material, or texture context.");

	const auto invalidPath = directory / "InvalidMaterialUvSets.gltf";
	bool rejected = false;
	try
	{
		static_cast<void>(LoadGltfScene(invalidPath));
	}
	catch (const std::runtime_error&)
	{
		rejected = true;
	}
	Expect(rejected, "Default import accepted a negative texture-coordinate set.");
	warnings.clear();
	const auto invalidScene = LoadGltfScene(invalidPath, fallback);
	const auto& material = invalidScene.materials.at(
		invalidScene.meshes.at(0).primitives.at(0).materialIndex.value());
	Expect(!material.metallicRoughness.baseColorTexture && !material.normalTexture &&
		material.metallicRoughness.metallicRoughnessTexture &&
		material.occlusionTexture && material.emissiveTexture,
		"Fallback failed to preserve unaffected texture usages.");
	Expect(warnings.size() == 2 &&
		warnings[0].find("negative texture-coordinate set") != std::string::npos &&
		warnings[1].find("only TEXCOORD_0") != std::string::npos,
		"Expected warnings for negative and unsupported texture-coordinate sets.");

	// No observer is required.
	static_cast<void>(LoadGltfScene(invalidPath, GltfImportOptions{
		.texturePolicy = GltfTexturePolicy::SkipUnusableTextures,
		.onWarning = {}
	}));
	warnings.clear();
	const auto validScene = LoadGltfScene(directory / "TriangleWithNormalMapWithoutTangents.gltf", fallback);
	Expect(warnings.empty() &&
		validScene.materials.at(0).normalTexture.has_value(),
		"Fallback changed a valid normal-mapped asset or emitted spurious warnings.");

	bool malformedRejected = false;
	try
	{
		static_cast<void>(LoadGltfScene(directory / "Invalid.gltf", fallback));
	}
	catch (const std::runtime_error&)
	{
		malformedRejected = true;
	}
	Expect(malformedRejected, "Texture fallback swallowed malformed-document errors.");
}
