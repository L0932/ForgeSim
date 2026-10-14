#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		[[nodiscard]] std::size_t ExpectedPixelDataSize(
			std::uint32_t width,
			std::uint32_t height)
		{
			constexpr std::size_t rgbaChannelCount = 4;

			return
				static_cast<std::size_t>(width) *
				static_cast<std::size_t>(height) *
				rgbaChannelCount;
		}
	}

	Texture2D::Texture2D(
		std::uint32_t width,
		std::uint32_t height,
		std::span<const std::uint8_t> rgbaPixels,
		TextureColorSpace colorSpace)
	{
		if (width == 0 || height == 0)
		{
			throw std::invalid_argument(
				"Texture dimensions must be positive.");
		}

		if (width >
			static_cast<std::uint32_t>(
				std::numeric_limits<GLsizei>::max()) ||
			height >
			static_cast<std::uint32_t>(
				std::numeric_limits<GLsizei>::max()))
		{
			throw std::invalid_argument(
				"Texture dimensions exceed OpenGL limits.");
		}

		const std::size_t expectedSize =
			ExpectedPixelDataSize(
				width,
				height);

		if (rgbaPixels.size() != expectedSize)
		{
			throw std::invalid_argument(
				"Texture pixel data does not match "
				"the RGBA8 dimensions.");
		}

		glCreateTextures(
			GL_TEXTURE_2D,
			1,
			&m_Handle);

		if (m_Handle == 0)
		{
			throw std::runtime_error(
				"Failed to create OpenGL texture.");
		}

		const std::uint32_t maximumDimension =
			std::max(width, height);

		const std::uint32_t mipLevelCount =
			std::bit_width(maximumDimension);

		const GLenum internalFormat =
			colorSpace == TextureColorSpace::Srgb
			? GL_SRGB8_ALPHA8
			: GL_RGBA8;

		glTextureStorage2D(
			m_Handle,
			static_cast<GLsizei>(mipLevelCount),
			internalFormat,
			static_cast<GLsizei>(width),
			static_cast<GLsizei>(height));

		glTextureSubImage2D(
			m_Handle,
			0,
			0,
			0,
			static_cast<GLsizei>(width),
			static_cast<GLsizei>(height),
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			rgbaPixels.data());

		glGenerateTextureMipmap(m_Handle);
	}

	Texture2D::~Texture2D()
	{
		if (m_Handle != 0)
		{
			glDeleteTextures(
				1,
				&m_Handle);
		}
	}

	Texture2D::Texture2D(Texture2D&& other) noexcept
		: m_Handle(std::exchange(other.m_Handle, 0))
	{}

	Texture2D& Texture2D::operator=(
		Texture2D&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		if (m_Handle != 0)
		{
			glDeleteTextures(1, &m_Handle);
		}

		m_Handle = std::exchange(
			other.m_Handle,
			0);

		return *this;
	}

	void Texture2D::Bind(
		std::uint32_t textureUnit) const noexcept
	{
		glBindTextureUnit(
			textureUnit,
			m_Handle);
	}
}