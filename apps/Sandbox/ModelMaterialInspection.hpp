#pragma once
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <array>
#include <cstdint>
#include <vector>

namespace ForgeSim::Sandbox
{
    // Borrowed for one UI frame; never retained across scene/resource changes.
    struct ModelMaterialInspection
    {
        const Assets::SceneAsset* asset = nullptr;
        // Base color, metallic/roughness, normal, occlusion, emissive.
        std::vector<std::array<std::uint32_t, 5>> textureHandles;
    };
}
