#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace ForgeSim::Assets
{
	enum class TextureFilter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	enum class TextureWrap
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge
	};

	struct ImageData
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::uint32_t channelCount = 0;

		std::vector<std::uint8_t> pixels;

		[[nodiscard]] bool IsValid() const noexcept
		{
			if (width == 0 ||
				height == 0 ||
				channelCount == 0)
			{
				return false;
			}

			const std::size_t expectedSize =
				static_cast<std::size_t>(width) *
				static_cast<std::size_t>(height) *
				static_cast<std::size_t>(channelCount);

			return pixels.size() == expectedSize;
		}
	};

	struct SamplerData
	{
		std::optional<TextureFilter> minificationFilter;
		std::optional<TextureFilter> magnificationFilter;

		TextureWrap wrapU = TextureWrap::Repeat;
		TextureWrap wrapV = TextureWrap::Repeat;
	};

	struct TextureData
	{
		std::size_t imageIndex = 0;
		std::optional<std::size_t> samplerIndex;
	};

	struct TextureResources
	{
		std::vector<ImageData> images;
		std::vector<SamplerData> samplers;
		std::vector<TextureData> textures;

		[[nodiscard]] bool HasValidReferences()
			const noexcept
		{
			for (const TextureData& texture : textures)
			{
				if (texture.imageIndex >= images.size())
				{
					return false;
				}

				if (texture.samplerIndex &&
					*texture.samplerIndex >= samplers.size())
				{
					return false;
				}
			}

			return true;
		}
	};
}