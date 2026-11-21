#pragma once
#include <ForgeSim/Assets/SceneAsset.hpp>
#include "TexturePreview.hpp"
#include <functional>
#include <cstdint>
#include <optional>
#include <vector>

namespace ForgeSim::Sandbox
{
    // Borrowed for one UI frame; never retained across scene/resource changes.
    struct ModelMaterialInspection
    {
        const Assets::SceneAsset* asset = nullptr;
        // Lazily creates a preview for a source image, independent of rendering textures.
        // The callback, like asset, is borrowed for the current UI frame only.
        std::function<std::uint32_t(std::size_t, TexturePreviewMode, TexturePreviewResolution)> texturePreview;
        std::uint64_t previewGeneration = 0;
        // Empty optional means all materials; an empty list means none for this selection.
        std::optional<std::vector<std::size_t>> materialIndices;
    };
}
