#pragma once
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <array>
#include <cmath>
#include <utility>
#include "SceneAssetPath.hpp"
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    // Reserved model reference, independent of the project's asset directory.
    [[nodiscard]] inline Assets::SceneAsset CreateBuiltinCubeAsset()
    {
        Assets::SceneAsset asset;
        asset.name = "Cube";
        asset.rootNodeIndices = {0};
        Assets::SceneNodeData node;
        node.name = "Cube";
        node.meshIndex = 0;
        asset.nodes.push_back(node);
        asset.materials.emplace_back();
        asset.materials.back().metallicRoughness.metallicFactor = 0.0f;
        asset.materials.back().metallicRoughness.roughnessFactor = 0.7f;
        Assets::MeshPrimitiveData primitive;
        primitive.materialIndex = 0;
        auto& mesh = primitive.geometry;
        mesh.availableAttributes = {.normals = true, .textureCoordinates = true, .tangents = true};
        const std::array<glm::vec3, 6> normals{{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}};
        const std::array<std::array<float,2>,4> uv{{{0,0},{1,0},{1,1},{0,1}}};
        for (const auto& normal : normals)
        {
            const glm::vec3 tangent = std::abs(normal.y) > 0.5f ? glm::vec3{1,0,0} : glm::normalize(glm::cross(glm::vec3{0,1,0}, normal));
            const auto bitangent = glm::cross(normal, tangent);
            const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
            for (const auto& coordinate : uv)
            {
                const auto position = normal * 0.5f + tangent * (coordinate[0] - 0.5f) + bitangent * (coordinate[1] - 0.5f);
                Assets::MeshVertex vertex;
                vertex.position = {position.x, position.y, position.z};
                vertex.normal = {normal.x, normal.y, normal.z};
                vertex.tangent = {tangent.x, tangent.y, tangent.z, 1.0f};
                vertex.textureCoordinates = coordinate;
                mesh.vertices.push_back(vertex);
            }
            mesh.indices.insert(mesh.indices.end(), {base,base+1,base+2,base,base+2,base+3});
        }
        Assets::SceneMeshData cube;
        cube.name = "Cube";
        cube.primitives.push_back(std::move(primitive));
        asset.meshes.push_back(std::move(cube));
        return asset;
    }
}
