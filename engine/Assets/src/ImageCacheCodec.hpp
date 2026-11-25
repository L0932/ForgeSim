#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <span>
#include <vector>

namespace ForgeSim::Assets::Detail
{
    inline constexpr std::uint64_t MaxDecodedCacheBytes = 256ull * 1024 * 1024;
    inline constexpr std::uint64_t ImageCacheMagic = 0x0000337643495346ull; // FSICv3
    // Little-endian uint64 fields: width, height, channels, CRC+dimensions,
    // magic/version, codec (0 raw, 1 LZ4), stored bytes, decoded bytes.
    using ImageCacheHeader = std::array<std::uint64_t, 8>;
    inline constexpr std::uint64_t ImageCacheHeaderBytes = 64;

    bool ReadImageCacheHeader(std::istream& input, ImageCacheHeader& header);
    bool WriteImageCacheHeader(std::ostream& output, const ImageCacheHeader& header);
    bool ValidateImageCacheHeader(const ImageCacheHeader& header, std::uint64_t fileBytes);

    // Empty result means raw fallback. Allocation/compression failure also falls
    // back to raw; a cache must never prevent source loading.
    std::vector<std::uint8_t> CompressCachePixels(std::span<const std::uint8_t> pixels);
    bool RestoreCachePixels(const ImageCacheHeader& header, std::span<const std::uint8_t> stored,
        std::vector<std::uint8_t>& pixels);
}
