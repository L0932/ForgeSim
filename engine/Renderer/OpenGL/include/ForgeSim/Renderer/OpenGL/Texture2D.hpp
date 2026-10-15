#pragma once

#include <cstdint>
#include <span>

namespace ForgeSim::Renderer::OpenGL
{
	enum class TextureColorSpace
	{
		Linear,
		Srgb
	};

	enum class Texture2DFormat
	{
		Rg16Float
	};

	class Texture2D final
	{
	public:

		// Creates an RGBA8 texture and uploads color image data.
		// The color space determines whether the GPU performs sRGB decoding.
		Texture2D(
			std::uint32_t width,
			std::uint32_t height,
			std::span<const std::uint8_t> rgbaPixels,
			TextureColorSpace colorSpace);

		// Creates an HDR texture and uploads linear RGB pixel data.
		Texture2D(
			std::uint32_t width,
			std::uint32_t height,
			std::span<const float> linearRgbPixels);

		// Allocates empty texture storage for GPU-generated content,
		// such as a rendered BRDF lookup table.
		Texture2D(
			std::uint32_t width,
			std::uint32_t height,
			Texture2DFormat format);

		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		Texture2D(Texture2D&& other) noexcept;
		Texture2D& operator=(Texture2D&& other) noexcept;

		void Bind(
			std::uint32_t textureUnit) const noexcept;

		void AttachToBoundFramebuffer() const noexcept;

	private:
		unsigned int m_Handle = 0;
	};
}