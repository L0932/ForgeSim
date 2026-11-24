#pragma once

#include <array>
#include <cstdint>
#include <span>
#ifdef FORGESIM_ACCELERATE_PNG
#include <libdeflate.h>
#endif

namespace ForgeSim::Assets::Detail
{
    inline constexpr auto ImageCrc32Table = []
    {
        std::array<std::uint32_t, 256> table{};
        for (std::uint32_t index = 0; index < table.size(); ++index)
        {
            auto value = index;
            for (unsigned bit = 0; bit < 8; ++bit)
                value = (value >> 1) ^ ((value & 1) ? 0xedb88320u : 0u);
            table[index] = value;
        }
        return table;
    }();

    // Same IEEE CRC-32 for accelerated and stb-only builds. This is an accidental
    // corruption check, not an authentication mechanism or an encoded cache key.
    inline std::uint32_t PortableImageCrc32(std::span<const std::uint8_t> bytes)
    {
        std::uint32_t crc = 0xffffffffu;
        for (auto byte : bytes) crc = ImageCrc32Table[(crc ^ byte) & 255] ^ (crc >> 8);
        return crc ^ 0xffffffffu;
    }

    inline std::uint32_t ImageCrc32(std::span<const std::uint8_t> bytes)
    {
#ifdef FORGESIM_ACCELERATE_PNG
        return libdeflate_crc32(0, bytes.data(), bytes.size());
#else
        return PortableImageCrc32(bytes);
#endif
    }

    inline std::uint64_t ImageCacheChecksum(std::span<const std::uint8_t> pixels,
        std::uint32_t width, std::uint32_t height)
    {
        return ImageCrc32(pixels) ^ (std::uint64_t{width} << 32) ^ height;
    }
}
