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

	class Texture2D final
	{
	public:
		Texture2D(
			std::uint32_t width,
			std::uint32_t height,
			std::span<const std::uint8_t> rgbaPixels,
			TextureColorSpace colorSpace);

		Texture2D(
			std::uint32_t width,
			std::uint32_t height,
			std::span<const float> linearRgbPixels);

		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		Texture2D(Texture2D&& other) noexcept;
		Texture2D& operator=(Texture2D&& other) noexcept;

		void Bind(
			std::uint32_t textureUnit) const noexcept;

	private:
		unsigned int m_Handle = 0;
	};
}