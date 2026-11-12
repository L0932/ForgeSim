#pragma once

#include <ForgeSim/Assets/SceneAsset.hpp>

#include <stdexcept>
#include <vector>

namespace ForgeSim::Sandbox
{
    // Keep material indices stable while avoiding GPU work for unused source
    // materials retained alongside the importer's texture-fallback variants.
    [[nodiscard]] inline std::vector<bool> ReferencedMaterials(const Assets::SceneAsset& scene)
    {
        std::vector<bool> used(scene.materials.size(), false);
        for (const auto& mesh : scene.meshes)
            for (const auto& primitive : mesh.primitives)
                if (primitive.materialIndex)
                {
                    if (*primitive.materialIndex >= used.size())
                        throw std::runtime_error("Primitive references an invalid material.");
                    used[*primitive.materialIndex] = true;
                }
        return used;
    }
}
