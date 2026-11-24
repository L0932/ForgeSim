#include <ForgeSim/Assets/MeshDisplacement.hpp>
#include <climits>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <stb_image.h>

namespace ForgeSim::Assets
{
    HeightField DecodeHeightPng(std::span<const std::uint8_t> encoded, unsigned channel, std::string_view sourceName)
    {
        constexpr std::uint8_t signature[]{137, 80, 78, 71, 13, 10, 26, 10};
        if (encoded.size() < 33 || encoded.size() > INT_MAX ||
            !std::equal(std::begin(signature), std::end(signature), encoded.begin()) ||
            (encoded[24] != 8 && encoded[24] != 16))
            throw std::invalid_argument("Height image must be an 8/16-bit PNG: " + std::string{sourceName});
        int width = 0, height = 0, channels = 0;
        const auto size = static_cast<int>(encoded.size());
        if (!stbi_info_from_memory(encoded.data(), size, &width, &height, &channels) || width <= 0 || height <= 0 || width > 16384 || height > 16384 ||
            static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height) > 16'777'216 || channel >= static_cast<unsigned>(channels))
            throw std::invalid_argument("Height PNG dimensions/channel are invalid (maximum 16M pixels): " + std::string{sourceName});
        const std::unique_ptr<stbi_us, decltype(&stbi_image_free)> pixels{
            stbi_load_16_from_memory(encoded.data(), size, &width, &height, &channels, 0), stbi_image_free};
        if (!pixels) throw std::runtime_error("Cannot decode height PNG: " + std::string{sourceName});
        HeightField result{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), {}};
        result.samples.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        for (std::size_t index = 0; index < result.samples.size(); ++index)
            result.samples[index] = static_cast<float>(pixels.get()[index * static_cast<std::size_t>(channels) + channel]) / 65535.0f;
        return result;
    }

    HeightField LoadHeightPng(const std::filesystem::path& path, unsigned channel)
    {
        std::ifstream input{path, std::ios::binary | std::ios::ate};
        if (!input) throw std::runtime_error("Cannot open height PNG: " + path.string());
        const auto length = input.tellg();
        if (length <= 0 || length > 256 * 1024 * 1024) throw std::runtime_error("Height PNG is empty or exceeds 256 MiB.");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
            throw std::runtime_error("Cannot read height PNG: " + path.string());
        return DecodeHeightPng(bytes, channel, path.string());
    }
}
