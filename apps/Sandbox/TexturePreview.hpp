#pragma once

#include <ForgeSim/Assets/TextureData.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace ForgeSim::Sandbox
{
    enum class TexturePreviewMode { Rgb, Roughness, Metallic };
    enum class TexturePreviewResolution { Thumbnail, Full };

    // Display source bytes directly. Color images already contain sRGB-encoded
    // RGB; data images show raw channel values. Alpha never hides RGB content.
    // No material factors, exposure, or tone mapping are applied to previews.
    [[nodiscard]] inline Assets::ImageData CreateTexturePreview(
        const Assets::ImageData& source, TexturePreviewMode mode,
        TexturePreviewResolution resolution = TexturePreviewResolution::Thumbnail)
    {
        const std::uint32_t maximumExtent = resolution == TexturePreviewResolution::Full
            ? std::max(source.width, source.height) : 512;
        if (source.width == 0 || source.height == 0 || source.channelCount != 4 ||
            static_cast<std::size_t>(source.width) >
                std::numeric_limits<std::size_t>::max() / 4 / source.height ||
            source.pixels.size() != static_cast<std::size_t>(source.width) * source.height * 4)
            throw std::invalid_argument("Texture previews require a valid RGBA8 image.");
        if (mode != TexturePreviewMode::Rgb && mode != TexturePreviewMode::Roughness &&
            mode != TexturePreviewMode::Metallic)
            throw std::invalid_argument("Unknown texture preview mode.");

        const auto extent = std::max(source.width, source.height);
        Assets::ImageData preview;
        preview.width = static_cast<std::uint32_t>(std::max<std::uint64_t>(1,
            static_cast<std::uint64_t>(source.width) * std::min(extent, maximumExtent) / extent));
        preview.height = static_cast<std::uint32_t>(std::max<std::uint64_t>(1,
            static_cast<std::uint64_t>(source.height) * std::min(extent, maximumExtent) / extent));
        preview.channelCount = 4;
        preview.pixels.resize(static_cast<std::size_t>(preview.width) * preview.height * 4);
        for (std::uint32_t y = 0; y < preview.height; ++y)
            for (std::uint32_t x = 0; x < preview.width; ++x)
            {
                // Nearest source sample at each preview texel center.
                const auto sx = (static_cast<std::uint64_t>(x) * 2 + 1) * source.width / (static_cast<std::uint64_t>(preview.width) * 2);
                const auto sy = (static_cast<std::uint64_t>(y) * 2 + 1) * source.height / (static_cast<std::uint64_t>(preview.height) * 2);
                const auto input = (static_cast<std::size_t>(sy) * source.width + static_cast<std::size_t>(sx)) * 4;
                const auto output = (static_cast<std::size_t>(y) * preview.width + x) * 4;
                for (std::size_t channel = 0; channel < 3; ++channel)
                    preview.pixels[output + channel] = source.pixels[input +
                        (mode == TexturePreviewMode::Rgb ? channel : mode == TexturePreviewMode::Roughness ? 1 : 2)];
                preview.pixels[output + 3] = 255;
            }
        return preview;
    }
}
