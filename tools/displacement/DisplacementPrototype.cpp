#include <ForgeSim/Assets/MeshDisplacement.hpp>
#include <ForgeSim/Assets/MeshProcessing.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace ForgeSim::Assets;
using Json = nlohmann::json;
namespace fs = std::filesystem;
namespace
{
    MeshData Plane()
    {
        MeshData mesh;
        for (const auto uv : {std::array<float, 2>{0, 0}, {1, 0}, {0, 1}, {1, 1}})
        {
            MeshVertex vertex;
            vertex.position = {uv[0] - .5f, 0, uv[1] - .5f};
            vertex.normal = {0, 1, 0};
            vertex.textureCoordinates = uv;
            mesh.vertices.push_back(vertex);
        }
        mesh.indices = {0, 2, 1, 1, 2, 3};
        mesh.availableAttributes.normals = true;
        mesh.availableAttributes.textureCoordinates = true;
        GenerateMissingNormalsAndTangents(mesh, true);
        return mesh;
    }
    HeightField SyntheticHeight()
    {
        HeightField image{128, 128, {}};
        for (unsigned y = 0; y < image.height; ++y)
            for (unsigned x = 0; x < image.width; ++x)
            {
                const float u = (static_cast<float>(x) + .5f) / static_cast<float>(image.width);
                const float v = (static_cast<float>(y) + .5f) / static_cast<float>(image.height);
                image.samples.push_back(.5f + .35f * std::sin(u * 12.5663706f) + .15f * std::sin(v * 12.5663706f));
            }
        return image;
    }
    unsigned Unsigned(const std::string& text)
    {
        unsigned result = 0;
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) throw std::invalid_argument("Expected unsigned integer.");
        return result;
    }
    float Float(const std::string& text)
    {
        std::size_t count = 0;
        const float result = std::stof(text, &count);
        if (count != text.size() || !std::isfinite(result)) throw std::invalid_argument("Expected finite number.");
        return result;
    }
    struct Glb
    {
        Json json = {{"asset", {{"version", "2.0"}, {"generator", "ForgeSim #132 CPU displacement prototype"}}},
            {"bufferViews", Json::array()}, {"accessors", Json::array()}, {"meshes", Json::array()}};
        std::vector<std::uint8_t> binary;
        std::size_t Attribute(const void* data, std::size_t bytes, std::size_t count, unsigned component, const char* type)
        {
            const auto offset = binary.size();
            const auto* first = static_cast<const std::uint8_t*>(data);
            binary.insert(binary.end(), first, first + bytes);
            const auto view = json["bufferViews"].size();
            json["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", offset}, {"byteLength", bytes}});
            const auto accessor = json["accessors"].size();
            json["accessors"].push_back({{"bufferView", view}, {"componentType", component}, {"count", count}, {"type", type}});
            return accessor;
        }
        void Add(const MeshData& mesh, const char* name)
        {
            std::vector<std::array<float, 3>> positions, normals;
            std::vector<std::array<float, 2>> uv;
            std::vector<std::array<float, 4>> tangents;
            auto minimum = mesh.vertices.front().position, maximum = minimum;
            for (const auto& vertex : mesh.vertices)
            {
                positions.push_back(vertex.position); normals.push_back(vertex.normal);
                uv.push_back(vertex.textureCoordinates); tangents.push_back(vertex.tangent);
                for (unsigned axis = 0; axis < 3; ++axis)
                {
                    minimum[axis] = std::min(minimum[axis], vertex.position[axis]);
                    maximum[axis] = std::max(maximum[axis], vertex.position[axis]);
                }
            }
            const auto position = Attribute(positions.data(), positions.size() * sizeof(positions[0]), positions.size(), 5126, "VEC3");
            json["accessors"][position]["min"] = minimum; json["accessors"][position]["max"] = maximum;
            const auto normal = Attribute(normals.data(), normals.size() * sizeof(normals[0]), normals.size(), 5126, "VEC3");
            const auto texcoord = Attribute(uv.data(), uv.size() * sizeof(uv[0]), uv.size(), 5126, "VEC2");
            const auto tangent = Attribute(tangents.data(), tangents.size() * sizeof(tangents[0]), tangents.size(), 5126, "VEC4");
            const auto indices = Attribute(mesh.indices.data(), mesh.indices.size() * sizeof(mesh.indices[0]), mesh.indices.size(), 5125, "SCALAR");
            json["meshes"].push_back({{"name", name}, {"primitives", Json::array({{
                {"attributes", {{"POSITION", position}, {"NORMAL", normal}, {"TEXCOORD_0", texcoord}, {"TANGENT", tangent}}},
                {"indices", indices}, {"material", 0}, {"mode", 4}}})}});
        }
        void Save(const fs::path& output, const MeshDisplacementSettings& settings)
        {
            static_assert(std::endian::native == std::endian::little);
            if (fs::exists(output)) throw std::runtime_error("Output exists; choose a new filename.");
            json["buffers"] = Json::array({{{"byteLength", binary.size()}}});
            json["materials"] = Json::array({{{"name", "Neutral prototype surface"}, {"doubleSided", true},
                {"pbrMetallicRoughness", {{"baseColorFactor", {.6f, .6f, .6f, 1.f}}, {"metallicFactor", 0}, {"roughnessFactor", .85f}}}}});
            json["nodes"] = Json::array({{{"mesh", 0}, {"name", "Flat control"}, {"translation", {-.65, 0, 0}}},
                {{"mesh", 1}, {"name", "CPU displaced"}, {"translation", {.65, 0, 0}}}});
            json["scenes"] = Json::array({{{"nodes", {0, 1}}}}); json["scene"] = 0;
            json["extras"] = {{"bakedPrototype", true}, {"scale", settings.scale}, {"midlevel", settings.midlevel},
                {"inverted", settings.inverted}, {"subdivisionLevels", settings.subdivisionLevels}};
            auto text = json.dump(); while (text.size() % 4) text += ' ';
            while (binary.size() % 4) binary.push_back(0);
            std::ofstream file{output, std::ios::binary};
            const auto word = [&](std::uint32_t value) { file.write(reinterpret_cast<const char*>(&value), 4); };
            word(0x46546c67); word(2); word(static_cast<std::uint32_t>(12 + 8 + text.size() + 8 + binary.size()));
            word(static_cast<std::uint32_t>(text.size())); word(0x4e4f534a); file.write(text.data(), static_cast<std::streamsize>(text.size()));
            word(static_cast<std::uint32_t>(binary.size())); word(0x004e4942);
            file.write(reinterpret_cast<const char*>(binary.data()), static_cast<std::streamsize>(binary.size()));
            file.close(); if (!file) throw std::runtime_error("Could not write prototype GLB.");
        }
    };
}
int main(int argc, char** argv)
{
    try
    {
        MeshDisplacementSettings settings; settings.subdivisionLevels = 6;
        settings.wrapU = settings.wrapV = TextureWrap::ClampToEdge;
        fs::path heightPath, output; unsigned channel = 0;
        for (int index = 1; index < argc; ++index)
        {
            const std::string argument = argv[index];
            if (argument == "--help")
            {
                std::cout << "--output result.glb [--height height.png] [--levels 0..8] [--scale .050] [--midlevel .225] [--invert] [--channel 0..3]\nWithout --height, use synthetic linear height data. Outputs flat/displaced 1-unit planes side by side.\n";
                return 0;
            }
            if (argument == "--invert") { settings.inverted = true; continue; }
            if (++index >= argc) throw std::invalid_argument("Missing argument value.");
            const std::string value = argv[index];
            if (argument == "--height") heightPath = fs::path{std::u8string(value.begin(), value.end())};
            else if (argument == "--output") output = fs::path{std::u8string(value.begin(), value.end())};
            else if (argument == "--levels") settings.subdivisionLevels = Unsigned(value);
            else if (argument == "--scale") settings.scale = Float(value);
            else if (argument == "--midlevel") settings.midlevel = Float(value);
            else if (argument == "--channel") channel = Unsigned(value);
            else throw std::invalid_argument("Unknown argument: " + argument);
        }
        if (output.empty()) throw std::invalid_argument("Specify --output with a new .glb filename.");
        const auto height = heightPath.empty() ? SyntheticHeight() : LoadHeightPng(heightPath, channel);
        const auto plane = Plane();
        const auto begin = std::chrono::steady_clock::now();
        const auto mesh = DisplaceMesh(plane, height, settings);
        const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
        Glb result; result.Add(plane, "Flat control"); result.Add(mesh, "CPU displaced"); result.Save(output, settings);
        std::cout << "Displaced vertices=" << mesh.vertices.size() << " triangles=" << mesh.indices.size()/3 <<
            " geometryBytes=" << mesh.vertices.size()*sizeof(MeshVertex)+mesh.indices.size()*sizeof(std::uint32_t) <<
            " generationMs=" << elapsed << '\n';
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
