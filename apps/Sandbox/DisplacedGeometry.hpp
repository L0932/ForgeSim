#pragma once
#include "MaterialDisplacement.hpp"
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/MeshDisplacement.hpp>

namespace ForgeSim::Sandbox
{
    // Geometry-only copy: rendering continues to borrow the original material/texture resources.
    // Every application starts from the source asset, never from previously displaced geometry.
    inline Assets::SceneAsset BuildDisplacedGeometry(const Assets::SceneAsset& source,
        const std::filesystem::path& model, const std::filesystem::path& root,
        const MaterialDisplacements& overrides)
    {
        Assets::SceneAsset result;
        result.name = source.name;
        result.nodes = source.nodes;
        result.rootNodeIndices = source.rootNodeIndices;
        result.meshes = source.meshes;
        std::size_t totalVertices = 0, totalTriangles = 0;
        for (const auto& [material, value] : overrides)
        {
            ValidateMaterialDisplacement(value);
            if (material >= source.materials.size()) throw std::invalid_argument("Height material index no longer exists.");
            if (!value.enabled || value.scale == 0) continue;
            if (source.heightGeometryBaked)
                throw std::invalid_argument("This GLB is already height-baked. Import the original GLB before enabling displacement.");
            const auto height = value.heightAsset.empty()
                ? Assets::LoadGltfHeightPng(model, *value.imageIndex, value.channel)
                : Assets::LoadHeightPng(root / std::filesystem::path{std::u8string{
                    value.heightAsset.begin(), value.heightAsset.end()}}, value.channel);
            bool matched = false;
            for (auto& mesh : result.meshes)
                for (auto& primitive : mesh.primitives)
                {
                    if (primitive.materialIndex != material) continue;
                    matched = true;
                    Assets::MeshDisplacementSettings settings;
                    settings.scale = value.scale;
                    settings.midlevel = value.midlevel;
                    settings.subdivisionLevels = value.subdivisions;
                    settings.inverted = value.inverted;
                    settings.stitchSmoothSeams = value.stitchSeams;
                    settings.wrapU = static_cast<Assets::TextureWrap>(value.wrapU);
                    settings.wrapV = static_cast<Assets::TextureWrap>(value.wrapV);
                    primitive.geometry = Assets::DisplaceMesh(primitive.geometry, height, settings);
                    totalVertices += primitive.geometry.vertices.size();
                    totalTriangles += primitive.geometry.indices.size() / 3;
                    if (totalVertices > 1'048'576 || totalTriangles > 524'288)
                        throw std::length_error("Object displacement exceeds its total geometry budget. Reduce subdivisions.");
                }
            if (!matched) throw std::invalid_argument("No primitives use the selected height material.");
        }
        return result;
    }
}
