#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/TextureTransform.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <nlohmann/json.hpp>

namespace
{
	using namespace ForgeSim::Assets;
	using Json = nlohmann::json;

	void Expect(bool condition, std::string_view message)
	{
		if (!condition) throw std::runtime_error(std::string{ message });
	}

	void ExpectUv(const std::array<float, 2>& actual,
		const std::array<float, 2>& expected)
	{
		for (std::size_t i = 0; i < actual.size(); ++i)
		{
			Expect(std::isfinite(actual[i]) && std::abs(actual[i] - expected[i]) < 1e-5f,
				"Unexpected transformed UV coordinate.");
		}
	}

	class TemporaryFixtures
	{
	public:
		TemporaryFixtures()
		{
			const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
			m_Directory = std::filesystem::temp_directory_path() /
				("ForgeSimTextureTransforms-" + std::to_string(stamp));
			if (!std::filesystem::create_directory(m_Directory))
				throw std::runtime_error("Cannot create unique temporary fixture directory.");
		}

		~TemporaryFixtures()
		{
			std::error_code ignored;
			std::filesystem::remove_all(m_Directory, ignored);
		}

		TemporaryFixtures(const TemporaryFixtures&) = delete;
		TemporaryFixtures& operator=(const TemporaryFixtures&) = delete;

		std::filesystem::path Write(const Json& document)
		{
			const auto path = m_Directory / (std::to_string(m_Next++) + ".gltf");
			std::ofstream stream{ path };
			stream << document.dump();
			stream.close();
			if (!stream) throw std::runtime_error("Cannot write temporary glTF fixture.");
			return path;
		}

	private:
		std::filesystem::path m_Directory;
		unsigned int m_Next = 0;
	};

	const MaterialData& MaterialFor(const SceneAsset& scene, std::size_t primitive)
	{
		return scene.materials.at(scene.meshes.at(0).primitives.at(primitive).materialIndex.value());
	}

	void ExpectImportError(const std::filesystem::path& path,
		const GltfImportOptions& options, std::string_view text)
	{
		try
		{
			static_cast<void>(LoadGltfScene(path, options));
		}
		catch (const std::runtime_error& error)
		{
			const std::string_view message{ error.what() };
			Expect(message.find(text) != std::string_view::npos,
				"Import error omitted the expected validation context.");
			Expect(message.find(path.string()) != std::string_view::npos,
				"Import error omitted the asset path.");
			return;
		}
		throw std::runtime_error("Expected invalid glTF import to fail.");
	}

	void CheckMath()
	{
		const std::array<float, 2> uv{ 0.25f, 0.5f };
		ExpectUv(ApplyTextureTransform({}, uv), uv);
		ExpectUv(ApplyTextureTransform(TextureTransform{
			.offset = { 2.0f, -3.0f } }, uv), { 2.25f, -2.5f });
		ExpectUv(ApplyTextureTransform(TextureTransform{
			.scale = { 7.0f, 7.0f } }, uv), { 1.75f, 3.5f });
		ExpectUv(ApplyTextureTransform(TextureTransform{
			.rotationRadians = std::numbers::pi_v<float> / 2.0f }, { 1.0f, 0.0f }),
			{ 0.0f, 1.0f });

		// Non-uniform scale makes an incorrect R/S ordering observable.
		const TextureTransform combined{
			.offset = { 0.25f, -0.5f },
			.rotationRadians = std::numbers::pi_v<float> / 2.0f,
			.scale = { 2.0f, 3.0f }
		};
		ExpectUv(ApplyTextureTransform(combined, uv), { -1.25f, 0.0f });
		const auto rows = CalculateTextureTransformRows(combined);
		ExpectUv({ rows[0][0], rows[0][1] }, { 0.0f, -3.0f });
		ExpectUv({ rows[1][0], rows[1][1] }, { 2.0f, 0.0f });
		ExpectUv({ rows[0][2], rows[1][2] }, combined.offset);
		ExpectUv(ApplyTextureTransform(TextureTransform{
			.offset = { 0.0f, 1.0f }, .scale = { 1.0f, -1.0f } }, { 0.25f, 0.2f }),
			{ 0.25f, 0.8f });
		ExpectUv(ApplyTextureTransform(TextureTransform{
			.offset = { -2.0f, 4.0f }, .scale = { 0.0f, 0.0f } }, uv),
			{ -2.0f, 4.0f });
		Expect(uv == std::array<float, 2>{ 0.25f, 0.5f }, "Source UVs were modified.");
		Expect(IsFiniteTextureTransform(combined), "Valid transform was rejected.");
		auto invalid = combined;
		invalid.rotationRadians = std::numeric_limits<float>::quiet_NaN();
		Expect(!IsFiniteTextureTransform(invalid), "NaN rotation was accepted.");
		invalid = combined;
		invalid.scale[0] = std::numeric_limits<float>::infinity();
		Expect(!IsFiniteTextureTransform(invalid), "Infinite scale was accepted.");
	}

	void CheckImport()
	{
		const auto fixturePath = std::filesystem::path{ FORGESIM_TEST_ASSET_DIRECTORY } /
			"TextureTransforms.gltf";
		const auto scene = LoadGltfScene(fixturePath);
		const auto visual = LoadGltfScene(FORGESIM_TEXTURE_TRANSFORM_VISUAL_ASSET);
		Expect(visual.meshes.size() == 11 && visual.materials.size() == 11 &&
			visual.textureResources.images.size() == 3,
			"Bundled visual texture-transform fixture failed to import.");
		const auto& material = MaterialFor(scene, 0);
		const auto& base = material.metallicRoughness.baseColorTexture.value();
		Expect(base.textureCoordinateSet == 0, "Extension texCoord did not override base texCoord.");
		ExpectUv(ApplyTextureTransform(base.transform, { 0.25f, 0.5f }), { -1.25f, 0.0f });
		ExpectUv(ApplyTextureTransform(material.metallicRoughness.metallicRoughnessTexture->transform,
			{ 0.25f, 0.5f }), { 0.5f, 2.0f });
		ExpectUv(ApplyTextureTransform(material.normalTexture->texture.transform,
			{ 0.25f, 0.5f }), { -0.25f, 1.0f });
		ExpectUv(ApplyTextureTransform(material.occlusionTexture->texture.transform,
			{ 0.25f, 0.5f }), { 0.5f, 0.375f });
		ExpectUv(ApplyTextureTransform(material.emissiveTexture->transform,
			{ 0.25f, 0.5f }), { 1.75f, 3.5f });
		Expect(material.normalTexture->scale == 0.6f && material.occlusionTexture->strength == 0.7f,
			"Texture transforms changed normal strength or occlusion strength.");
		Expect(scene.meshes[0].primitives[0].materialIndex == scene.meshes[0].primitives[1].materialIndex,
			"Strict import changed shared material assignment.");
		ExpectUv(scene.meshes[0].primitives[0].geometry.vertices.at(1).textureCoordinates, { 1.0f, 0.0f });

		std::vector<std::string> warnings;
		const GltfImportOptions fallback{
			.texturePolicy = GltfTexturePolicy::SkipUnusableTextures,
			.onWarning = [&warnings](std::string_view message) { warnings.emplace_back(message); }
		};
		const auto fallbackScene = LoadGltfScene(fixturePath, fallback);
		Expect(warnings.size() == 5, "Expected missing-UV warnings only for the second primitive.");
		Expect(MaterialFor(fallbackScene, 0).metallicRoughness.baseColorTexture.has_value() &&
			!MaterialFor(fallbackScene, 1).metallicRoughness.baseColorTexture,
			"Effective UV selection broke per-primitive material fallback.");
		ExpectUv(ApplyTextureTransform(MaterialFor(fallbackScene, 0).emissiveTexture->transform,
			{ 0.25f, 0.5f }), { 1.75f, 3.5f });

		std::ifstream input{ fixturePath };
		const Json original = Json::parse(input);
		TemporaryFixtures temporary;
		const Json::json_pointer basePointer{ "/materials/0/pbrMetallicRoughness/baseColorTexture" };

		auto required = original;
		required["extensionsRequired"].push_back("KHR_materials_transmission");
		required["extensionsUsed"].push_back("KHR_materials_transmission");
		const auto requiredPath = temporary.Write(required);
		ExpectImportError(requiredPath, {}, "Unsupported required glTF extension 'KHR_materials_transmission'");
		ExpectImportError(requiredPath, fallback, "Unsupported required glTF extension 'KHR_materials_transmission'");
		// An unsupported optional extension can still use its authored core
		// material fallback; it must not be mistaken for a supported feature.
		required["extensionsRequired"] = original["extensionsRequired"];
		static_cast<void>(LoadGltfScene(temporary.Write(required)));

		// An explicit zero override also supersedes a negative base set.
		auto document = original;
		document[basePointer]["texCoord"] = -1;
		const auto overriddenPath = temporary.Write(document);
		Expect(MaterialFor(LoadGltfScene(overriddenPath), 0).metallicRoughness.baseColorTexture->textureCoordinateSet == 0,
			"Valid override failed to supersede negative base texCoord.");

		// Without an override, the base texCoord remains authoritative.
		document = original;
		document[basePointer]["extensions"]["KHR_texture_transform"].erase("texCoord");
		const auto baseSetPath = temporary.Write(document);
		Expect(MaterialFor(LoadGltfScene(baseSetPath), 0).metallicRoughness.baseColorTexture->textureCoordinateSet == 1,
			"Missing override did not preserve the base UV set.");
		warnings.clear();
		const auto skipped = LoadGltfScene(baseSetPath, fallback);
		Expect(!MaterialFor(skipped, 0).metallicRoughness.baseColorTexture &&
			MaterialFor(skipped, 0).normalTexture.has_value(), "Unsupported set fallback removed unaffected textures.");
		Expect(warnings.at(0).find("TEXCOORD_1") != std::string::npos,
			"Warning reported the wrong effective UV set.");

		document = original;
		document[basePointer]["texCoord"] = 0;
		document[basePointer]["extensions"]["KHR_texture_transform"]["texCoord"] = 1;
		const auto unsupportedPath = temporary.Write(document);
		Expect(MaterialFor(LoadGltfScene(unsupportedPath), 0).metallicRoughness.baseColorTexture->textureCoordinateSet == 1,
			"Nonzero override failed to supersede base set zero.");
		warnings.clear();
		const auto unsupported = LoadGltfScene(unsupportedPath, fallback);
		Expect(!MaterialFor(unsupported, 0).metallicRoughness.baseColorTexture &&
			MaterialFor(unsupported, 0).normalTexture.has_value() &&
			warnings.at(0).find("TEXCOORD_1") != std::string::npos,
			"Fallback ignored an unsupported extension override.");

		document = original;
		document[basePointer]["texCoord"] = 0;
		document[basePointer]["extensions"]["KHR_texture_transform"]["texCoord"] = -1;
		const auto negativePath = temporary.Write(document);
		ExpectImportError(negativePath, {}, "negative texture-coordinate set");
		warnings.clear();
		const auto negative = LoadGltfScene(negativePath, fallback);
		Expect(!MaterialFor(negative, 0).metallicRoughness.baseColorTexture &&
			warnings.at(0).find("TEXCOORD_-1") != std::string::npos,
			"Fallback did not honor a negative extension override.");

		// Both absent and empty extensions use identity/default values.
		for (const bool emptyExtension : { false, true })
		{
			document = original;
			auto& texture = document[basePointer];
			texture["texCoord"] = 0;
			if (emptyExtension) texture["extensions"]["KHR_texture_transform"] = Json::object();
			else texture.erase("extensions");
			const auto defaults = LoadGltfScene(temporary.Write(document));
			ExpectUv(ApplyTextureTransform(MaterialFor(defaults, 0).metallicRoughness.baseColorTexture->transform,
				{ 0.25f, 0.5f }), { 0.25f, 0.5f });
		}

		// Legal JSON numbers may exceed float range. Never hide malformed
		// transforms behind the optional missing/unsupported-UV fallback.
		for (const char* component : { "/offset/0", "/offset/1", "/rotation", "/scale/0", "/scale/1" })
		{
			document = original;
			auto& transform = document[basePointer]["extensions"]["KHR_texture_transform"];
			transform[Json::json_pointer{ component }] = 1e100;
			transform["texCoord"] = 1;
			const auto invalidPath = temporary.Write(document);
			ExpectImportError(invalidPath, {}, "KHR_texture_transform");
			ExpectImportError(invalidPath, fallback, "KHR_texture_transform");
		}
	}
}

int main()
{
	try
	{
		CheckMath();
		CheckImport();
		return EXIT_SUCCESS;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return EXIT_FAILURE;
	}
}
