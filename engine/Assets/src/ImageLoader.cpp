#include <ForgeSim/Assets/ImageLoader.hpp>

#include <climits>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ForgeSim::Assets
{
	namespace
	{
		constexpr std::uint32_t decodedChannelCount = 4;
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
			decodedChannelCount;

		ImageData image{
			.width = static_cast<std::uint32_t>(width),
			.height = static_cast<std::uint32_t>(height),
			.channelCount = decodedChannelCount,
			.pixels = std::vector<std::uint8_t>{
				decodedPixels,
				decodedPixels + pixelDataSize
			}
		};

		stbi_image_free(decodedPixels);

		return image;
	}
}