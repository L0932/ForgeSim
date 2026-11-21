#include <ForgeSim/Assets/GltfLoader.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace
{
    using namespace ForgeSim::Assets;

    void Require(bool value, const char* message)
    {
        if (!value) throw std::runtime_error(message);
    }

    void CheckMaterials(const SceneAsset& scene)
    {
        Require(scene.materials.size() == 4 && scene.meshes.size() == 4,
            "Expected four mixed lit/unlit material panels.");
        constexpr std::array<std::array<float, 4>, 4> factors{{
            {1, 1, 1, 1}, {.5f, .75f, 1, 1}, {1, .5f, .25f, 1}, {.25f, .5f, 1, .5f}}};
        constexpr AlphaMode modes[]{AlphaMode::Opaque, AlphaMode::Opaque, AlphaMode::Mask, AlphaMode::Blend};
        for (std::size_t i = 0; i < 4; ++i)
        {
            const auto& primitive = scene.meshes.at(i).primitives.at(0);
            Require(primitive.materialIndex == i, "Primitive lost its authored material assignment.");
            const auto& material = scene.materials.at(i);
            Require(material.unlit == (i != 0), "Unlit must be read per material, not per asset.");
            Require(material.metallicRoughness.baseColorFactor == factors[i] &&
                material.alphaMode == modes[i] && material.doubleSided == (i != 0),
                "Unlit import changed base color, alpha mode or sidedness.");
            Require(std::abs(material.alphaCutoff - (i == 2 ? .6f : .5f)) < 1e-6f,
                "Unlit alpha cutoff was not preserved.");
            const auto& texture = material.metallicRoughness.baseColorTexture;
            Require(texture && texture->textureIndex == 0 && texture->textureCoordinateSet == 0,
                "Unlit base-color texture or primary UV binding was lost.");
            const auto& geometry = primitive.geometry;
            Require(geometry.vertices.size() == 4 && geometry.indices.size() == 6 &&
                geometry.availableAttributes.colors && geometry.availableAttributes.textureCoordinates,
                "Unlit fixture lost its vertex colors or texture coordinates.");
            constexpr std::array<std::array<float, 2>, 4> uvs{{{0,0}, {1,0}, {1,1}, {0,1}}};
            for (std::size_t vertex = 0; vertex < 4; ++vertex)
                Require(geometry.vertices[vertex].color == std::array{.5f, 1.0f, .75f} &&
                    geometry.vertices[vertex].textureCoordinates == uvs[vertex],
                    "Unlit import modified authored vertex color or UV values.");
        }
        Require(scene.textureResources.images.size() == 1 && scene.textureResources.HasValidReferences(),
            "Expected a valid shared embedded texture.");
        const auto& image = scene.textureResources.images.at(0);
        Require(image.width == 2 && image.height == 2 && image.channelCount == 4 && image.IsValid(),
            "Embedded alpha test image did not decode correctly.");
        Require(image.pixels[3] == 255 && image.pixels[7] == 0 && image.pixels[11] == 128 &&
            image.pixels[15] == 255, "Opaque, transparent and partial texture alpha must survive decoding.");
    }

    struct TemporaryDirectory
    {
        std::filesystem::path path;
        TemporaryDirectory()
        {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            path = std::filesystem::temp_directory_path() / ("ForgeSim-unlit-" + std::to_string(stamp));
            if (!std::filesystem::create_directory(path))
                throw std::runtime_error("Could not create unlit test directory.");
        }
        ~TemporaryDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void WriteUint32(std::ostream& output, std::uint32_t value)
    {
        const char bytes[]{static_cast<char>(value & 255u), static_cast<char>((value >> 8) & 255u),
            static_cast<char>((value >> 16) & 255u), static_cast<char>((value >> 24) & 255u)};
        output.write(bytes, 4);
    }
}

void RunGltfUnlitTests()
{
    using namespace ForgeSim::Assets;
    const std::filesystem::path assets{FORGESIM_TEST_ASSET_DIRECTORY};
    const auto fixture = assets / "UnlitMaterials.gltf";
    Require(!MaterialData{}.unlit, "Ordinary material defaults must remain lit.");
    CheckMaterials(LoadGltfScene(fixture));

    std::ifstream source(fixture, std::ios::binary);
    Require(static_cast<bool>(source), "Could not read unlit fixture.");
    auto document = nlohmann::json::parse(source);
    TemporaryDirectory temporary;

    // The mesh-only API intentionally accepts exactly one mesh/primitive.
    // Reuse its existing triangle fixture and add one required unlit material.
    std::ifstream triangleSource(assets / "Triangle.gltf", std::ios::binary);
    Require(static_cast<bool>(triangleSource), "Could not read triangle fixture.");
    auto triangle = nlohmann::json::parse(triangleSource);
    triangle["extensionsUsed"] = {"KHR_materials_unlit"};
    triangle["extensionsRequired"] = {"KHR_materials_unlit"};
    triangle["materials"] = nlohmann::json::array({
        {{"extensions", {{"KHR_materials_unlit", nlohmann::json::object()}}}}});
    triangle.at("meshes").at(0).at("primitives").at(0)["material"] = 0;
    const auto trianglePath = temporary.path / "UnlitTriangle.gltf";
    {
        std::ofstream output(trianglePath, std::ios::binary);
        output << triangle.dump();
        Require(static_cast<bool>(output), "Could not write unlit triangle fixture.");
    }
    Require(LoadGltfMesh(trianglePath).vertices.size() == 3,
        "Mesh loading must accept required unlit extensions too.");

    // Wrap the same self-contained document in a JSON-only GLB container.
    // The geometry/image data URIs remain valid; no external files are needed.
    auto padded = document.dump();
    while (padded.size() % 4 != 0) padded += ' ';
    const auto glbPath = temporary.path / "UnlitMaterials.glb";
    {
        std::ofstream glb(glbPath, std::ios::binary);
        WriteUint32(glb, 0x46546c67u); // glTF magic
        WriteUint32(glb, 2);
        WriteUint32(glb, static_cast<std::uint32_t>(20 + padded.size()));
        WriteUint32(glb, static_cast<std::uint32_t>(padded.size()));
        WriteUint32(glb, 0x4e4f534au); // JSON chunk
        glb.write(padded.data(), static_cast<std::streamsize>(padded.size()));
        Require(static_cast<bool>(glb), "Could not write temporary unlit GLB.");
    }
    CheckMaterials(LoadGltfScene(glbPath));

    // Optional unlit extensions must still be honored on individual materials.
    Require(document.contains("extensionsRequired"), "Unlit fixture must require the extension.");
    document.erase("extensionsRequired");
    const auto optionalPath = temporary.path / "OptionalUnlit.gltf";
    {
        std::ofstream optional(optionalPath, std::ios::binary);
        optional << document.dump();
        Require(static_cast<bool>(optional), "Could not write optional unlit fixture.");
    }
    CheckMaterials(LoadGltfScene(optionalPath));

    bool rejected = false;
    try
    {
        static_cast<void>(LoadGltfScene(assets / "UnsupportedRequiredExtension.gltf"));
    }
    catch (const std::runtime_error& error)
    {
        rejected = std::string{error.what()}.find(
            "Unsupported required glTF extension 'FORGESIM_test_unsupported'") != std::string::npos;
    }
    Require(rejected, "Supporting unlit must not disable unknown required-extension validation.");
}
