#include <ForgeSim/Assets/ImageLoader.hpp>

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef FORGESIM_ACCELERATE_PNG
#include <libdeflate.h>
#define STBI_NO_ZLIB
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef FORGESIM_ACCELERATE_PNG
// stb's PNG reader calls this whole-buffer inflate entry point. Keep its PNG
// parser, filtering, transparency, channel and bit-depth conversions unchanged.
// Allocations use malloc/free, matching this translation unit's STBI allocator.
extern "C" char* stbi_zlib_decode_malloc_guesssize_headerflag(
    const char* buffer, int length, int initialSize, int* outputLength, int parseHeader)
{
    if (!buffer || length <= 0 || initialSize < 0)
    {
        stbi__err("bad zlib input", "Corrupt PNG");
        return nullptr;
    }
    const std::unique_ptr<libdeflate_decompressor, decltype(&libdeflate_free_decompressor)> decoder{
        libdeflate_alloc_decompressor(), libdeflate_free_decompressor};
    if (!decoder) { stbi__err("outofmem", "Out of memory"); return nullptr; }
    auto capacity = static_cast<std::size_t>(std::max(initialSize, 1));
    for (;;)
    {
        std::unique_ptr<char, decltype(&std::free)> output{
            static_cast<char*>(std::malloc(capacity)), std::free};
        if (!output) { stbi__err("outofmem", "Out of memory"); return nullptr; }
        std::size_t actual = 0;
        const auto result = parseHeader
            ? libdeflate_zlib_decompress(decoder.get(), buffer, static_cast<std::size_t>(length),
                output.get(), capacity, &actual)
            : libdeflate_deflate_decompress(decoder.get(), buffer, static_cast<std::size_t>(length),
                output.get(), capacity, &actual);
        if (result == LIBDEFLATE_SUCCESS)
        {
            if (outputLength) *outputLength = static_cast<int>(actual);
            return output.release();
        }
        if (result != LIBDEFLATE_INSUFFICIENT_SPACE || capacity == static_cast<std::size_t>(INT_MAX))
        {
            stbi__err("bad zlib data", "Corrupt PNG");
            return nullptr;
        }
        // Adam7/packed rows may exceed stb's initial estimate. Grow only within
        // stb's existing signed-int output-length contract.
        capacity = std::min(capacity * 2, static_cast<std::size_t>(INT_MAX));
    }
}
#endif

namespace ForgeSim::Assets
{
	namespace
	{
		constexpr std::uint32_t decodedLdrChannelCount = 4;
		constexpr std::uint32_t decodedHdrChannelCount = 3;
	}

	ImageData LoadImage(
		const std::filesystem::path& path)
	{
		std::ifstream file{
			path,
			std::ios::binary | std::ios::ate
		};

		if (!file)
		{
			throw std::runtime_error(
				"Image file not found: " +
				path.string());
		}

		const std::streampos endPosition =
			file.tellg();

		if (endPosition <= 0)
		{
			throw std::runtime_error(
				"Image file is empty or unreadable: " +
				path.string());
		}

		const auto encodedSize =
			static_cast<std::size_t>(endPosition);

		std::vector<std::uint8_t> encodedData(
			encodedSize);

		file.seekg(0, std::ios::beg);

		file.read(
			reinterpret_cast<char*>(
				encodedData.data()),
			static_cast<std::streamsize>(
				encodedData.size()));

		if (!file)
		{
			throw std::runtime_error(
				"Failed while reading image file: " +
				path.string());
		}

		return DecodeImage(
			encodedData,
			path.string());
	}

	ImageData DecodeImage(
		std::span<const std::uint8_t> encodedData,
		std::string_view sourceName)
	{
		if (encodedData.empty())
		{
			throw std::runtime_error(
				"Image data is empty: " +
				std::string{ sourceName });
		}

		if (encodedData.size() >
			static_cast<std::size_t>(INT_MAX))
		{
			throw std::runtime_error(
				"Image data is too large to decode: " +
				std::string{ sourceName });
		}

		int width = 0;
		int height = 0;
		int sourceChannelCount = 0;

		stbi_uc* decodedPixels =
			stbi_load_from_memory(
				encodedData.data(),
				static_cast<int>(
					encodedData.size()),
				&width,
				&height,
				&sourceChannelCount,
				STBI_rgb_alpha);

		if (decodedPixels == nullptr)
		{
			const char* failureReason =
				stbi_failure_reason();

			throw std::runtime_error(
				"Failed to decode image '" +
				std::string{ sourceName } +
				"': " +
				(failureReason != nullptr
					? failureReason
					: "unknown decoding error"));
		}

		if (width <= 0 || height <= 0)
		{
			stbi_image_free(decodedPixels);

			throw std::runtime_error(
				"Decoded image has invalid dimensions: " +
				std::string{ sourceName });
		}

		const std::size_t pixelDataSize =
			static_cast<std::size_t>(width) *
			static_cast<std::size_t>(height) *
			decodedLdrChannelCount;

		ImageData image{
			.width = static_cast<std::uint32_t>(width),
			.height = static_cast<std::uint32_t>(height),
			.channelCount = decodedLdrChannelCount,
			.pixels = std::vector<std::uint8_t>{
				decodedPixels,
				decodedPixels + pixelDataSize
			}
		};

		stbi_image_free(decodedPixels);

		return image;
	}

	HdrImageData LoadHdrImage(
		const std::filesystem::path& path)
	{
		std::ifstream file{
			path,
			std::ios::binary | std::ios::ate
		};

		if (!file)
		{
			throw std::runtime_error(
				"HDR image file not found: " +
				path.string());
		}

		const std::streampos endPosition =
			file.tellg();

		if (endPosition <= 0)
		{
			throw std::runtime_error(
				"HDR image file is empty or unreadable: " +
				path.string());
		}

		const auto encodedSize =
			static_cast<std::size_t>(
				endPosition);

		std::vector<std::uint8_t> encodedData(
			encodedSize);

		file.seekg(
			0,
			std::ios::beg);

		file.read(
			reinterpret_cast<char*>(
				encodedData.data()),
			static_cast<std::streamsize>(
				encodedData.size()));

		if (!file)
		{
			throw std::runtime_error(
				"Failed while reading HDR image file: " +
				path.string());
		}

		return DecodeHdrImage(
			encodedData,
			path.string());
	}

	HdrImageData DecodeHdrImage(
		std::span<const std::uint8_t> encodedData,
		std::string_view sourceName)
	{
		if (encodedData.empty())
		{
			throw std::runtime_error(
				"HDR image data is empty: " +
				std::string{ sourceName });
		}

		if (encodedData.size() >
			static_cast<std::size_t>(INT_MAX))
		{
			throw std::runtime_error(
				"HDR image data is too large to decode: " +
				std::string{ sourceName });
		}

		const int encodedSize =
			static_cast<int>(
				encodedData.size());

		if (stbi_is_hdr_from_memory(
			encodedData.data(),
			encodedSize) == 0)
		{
			throw std::runtime_error(
				"Image is not encoded as HDR data: " +
				std::string{ sourceName });
		}

		int width = 0;
		int height = 0;
		int sourceChannelCount = 0;

		float* decodedPixels =
			stbi_loadf_from_memory(
				encodedData.data(),
				encodedSize,
				&width,
				&height,
				&sourceChannelCount,
				STBI_rgb);

		if (decodedPixels == nullptr)
		{
			const char* failureReason =
				stbi_failure_reason();

			throw std::runtime_error(
				"Failed to decode HDR image '" +
				std::string{ sourceName } +
				"': " +
				(failureReason != nullptr
					? failureReason
					: "unknown decoding error"));
		}

		if (width <= 0 || height <= 0)
		{
			stbi_image_free(decodedPixels);

			throw std::runtime_error(
				"Decoded HDR image has invalid dimensions: " +
				std::string{ sourceName });
		}

		const std::size_t pixelDataSize =
			static_cast<std::size_t>(width) *
			static_cast<std::size_t>(height) *
			decodedHdrChannelCount;

		HdrImageData image{
			.width =
				static_cast<std::uint32_t>(width),

			.height =
				static_cast<std::uint32_t>(height),

			.channelCount =
				decodedHdrChannelCount,

			.pixels = std::vector<float>{
				decodedPixels,
				decodedPixels + pixelDataSize
			}
		};

		stbi_image_free(decodedPixels);

		return image;
	}
}