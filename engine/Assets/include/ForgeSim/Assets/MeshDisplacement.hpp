#pragma once

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/TextureData.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>

namespace ForgeSim::Assets
{
    // One channel of normalized linear data, row zero at the top of the image.
    struct HeightField
    {
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::vector<float> samples;
    };

    // PNG only in this first prototype: retain native 8/16-bit values without sRGB conversion.
    [[nodiscard]] HeightField DecodeHeightPng(std::span<const std::uint8_t> encoded,
        unsigned channel = 0, std::string_view sourceName = "<memory>");
    [[nodiscard]] HeightField LoadHeightPng(const std::filesystem::path& path, unsigned channel = 0);

    struct MeshDisplacementSettings
    {
        bool enabled = true;
        float scale = 0.050f;
        float midlevel = 0.225f;
        bool inverted = false;
        // Average displacement and area normals for exact coincident positions
        // with matching source normals. Preserve UV splits and hard edges.
        bool stitchSmoothSeams = false;
        unsigned subdivisionLevels = 0;
        TextureWrap wrapU = TextureWrap::Repeat;
        TextureWrap wrapV = TextureWrap::Repeat;
        std::size_t maxVertices = 1'048'576;
        std::size_t maxTriangles = 524'288;
    };

    // Returns independent geometry. Displacement is along the interpolated source
    // normal in mesh-local units, before node/object transforms. UV indices remain
    // separate even when stitchSmoothSeams joins their positions and normals. Disabled/zero scale returns an exact copy without subdivision.
    [[nodiscard]] MeshData DisplaceMesh(const MeshData& source, const HeightField& height,
        const MeshDisplacementSettings& settings = {});

    [[nodiscard]] float SampleHeight(const HeightField& height, float u, float v,
        TextureWrap wrapU = TextureWrap::Repeat, TextureWrap wrapV = TextureWrap::Repeat);
}
