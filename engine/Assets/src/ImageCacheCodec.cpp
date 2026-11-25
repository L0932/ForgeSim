#include "ImageCacheCodec.hpp"

#include <lz4.h>
#include <algorithm>
#include <istream>
#include <ostream>
#include <new>

namespace ForgeSim::Assets::Detail
{
    bool ReadImageCacheHeader(std::istream& input, ImageCacheHeader& header)
    {
        std::array<unsigned char, ImageCacheHeaderBytes> bytes{};
        if (!input.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) return false;
        header.fill(0);
        for (std::size_t field = 0; field < header.size(); ++field)
            for (unsigned byte = 0; byte < 8; ++byte)
                header[field] |= std::uint64_t{bytes[field * 8 + byte]} << (byte * 8);
        return true;
    }

    bool WriteImageCacheHeader(std::ostream& output, const ImageCacheHeader& header)
    {
        std::array<unsigned char, ImageCacheHeaderBytes> bytes{};
        for (std::size_t field = 0; field < header.size(); ++field)
            for (unsigned byte = 0; byte < 8; ++byte)
                bytes[field * 8 + byte] = static_cast<unsigned char>(header[field] >> (byte * 8));
        return static_cast<bool>(output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    }

    bool ValidateImageCacheHeader(const ImageCacheHeader& h, std::uint64_t fileBytes)
    {
        // Divide before multiplying: no untrusted dimensions may overflow.
        if (h[0] == 0 || h[1] == 0 || h[2] != 4 || h[4] != ImageCacheMagic ||
            h[1] > MaxDecodedCacheBytes / 4 || h[0] > MaxDecodedCacheBytes / 4 / h[1]) return false;
        const auto decoded = h[0] * h[1] * 4;
        if (h[7] != decoded || h[6] == 0 || h[6] > decoded ||
            fileBytes != ImageCacheHeaderBytes + h[6]) return false;
        return (h[5] == 0 && h[6] == decoded) || (h[5] == 1 && h[6] < decoded);
    }

    std::vector<std::uint8_t> CompressCachePixels(std::span<const std::uint8_t> pixels)
    {
        if (pixels.size() < 64 * 1024 || pixels.size() > MaxDecodedCacheBytes) return {};
        // Require >=12.5% and >=4 KiB saved; cap the output allocation to that
        // threshold so incompressible data falls back without an expanded payload.
        const auto minimumSaving = std::max<std::size_t>(4096, pixels.size() / 8);
        try
        {
            std::vector<std::uint8_t> encoded(pixels.size() - minimumSaving);
            const auto count = LZ4_compress_fast(reinterpret_cast<const char*>(pixels.data()),
                reinterpret_cast<char*>(encoded.data()), static_cast<int>(pixels.size()),
                static_cast<int>(encoded.size()), 1);
            if (count <= 0) return {};
            encoded.resize(static_cast<std::size_t>(count));
            return encoded;
        }
        catch (const std::bad_alloc&) { return {}; }
    }

    bool RestoreCachePixels(const ImageCacheHeader& h, std::span<const std::uint8_t> stored,
        std::vector<std::uint8_t>& pixels)
    {
        if (!ValidateImageCacheHeader(h, ImageCacheHeaderBytes + stored.size()) || h[6] != stored.size())
            return false;
        pixels.resize(static_cast<std::size_t>(h[7]));
        if (h[5] == 0)
        {
            std::copy(stored.begin(), stored.end(), pixels.begin());
            return true;
        }
        return LZ4_decompress_safe(reinterpret_cast<const char*>(stored.data()),
            reinterpret_cast<char*>(pixels.data()), static_cast<int>(stored.size()),
            static_cast<int>(pixels.size())) == static_cast<int>(pixels.size());
    }
}
