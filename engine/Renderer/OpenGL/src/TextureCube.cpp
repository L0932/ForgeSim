#include <ForgeSim/Renderer/OpenGL/TextureCube.hpp>

#include <bit>
#include <limits>
#include <stdexcept>
#include <utility>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	TextureCube::TextureCube(
		std::uint32_t size,
		std::uint32_t mipLevelCount)
		: m_Size(size)
		, m_MipLevelCount(mipLevelCount)
	{
		if (size == 0)
		{
			throw std::invalid_argument(
				"Cubemap size must be positive.");
		}

		if (size >
			static_cast<std::uint32_t>(
				std::numeric_limits<GLsizei>::max()))
		{
			throw std::invalid_argument(
				"Cubemap size exceeds OpenGL limits.");
		}

		const std::uint32_t maximumMipLevelCount =
			std::bit_width(size);

		if (mipLevelCount == 0 ||
			mipLevelCount > maximumMipLevelCount)
		{
			throw std::invalid_argument(
				"Cubemap mip-level count is invalid.");
		}

		glCreateTextures(
			GL_TEXTURE_CUBE_MAP,
			1,
			&m_Handle);

		if (m_Handle == 0)
		{
			throw std::runtime_error(
				"Failed to create OpenGL cubemap.");
		}

		glTextureStorage2D(
			m_Handle,
			static_cast<GLsizei>(mipLevelCount),
			GL_RGB16F,
			static_cast<GLsizei>(size),
			static_cast<GLsizei>(size));

		glTextureParameteri(
			m_Handle,
			GL_TEXTURE_MIN_FILTER,
			mipLevelCount > 1
			? GL_LINEAR_MIPMAP_LINEAR
			: GL_LINEAR);

		glTextureParameteri(
			m_Handle,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);

		glTextureParameteri(
			m_Handle,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);

		glTextureParameteri(
			m_Handle,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);

		glTextureParameteri(
			m_Handle,
			GL_TEXTURE_WRAP_R,
			GL_CLAMP_TO_EDGE);
	}

	TextureCube::~TextureCube()
	{
		if (m_Handle != 0)
		{
			glDeleteTextures(
				1,
				&m_Handle);
		}
	}

	TextureCube::TextureCube(
		TextureCube&& other) noexcept
		: m_Handle(
			std::exchange(
				other.m_Handle,
				0))
		, m_Size(
			std::exchange(
				other.m_Size,
				0))
		, m_MipLevelCount(
			std::exchange(
				other.m_MipLevelCount,
				0))
	{}

	TextureCube& TextureCube::operator=(
		TextureCube&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		if (m_Handle != 0)
		{
			glDeleteTextures(
				1,
				&m_Handle);
		}

		m_Handle =
			std::exchange(
				other.m_Handle,
				0);

		m_Size =
			std::exchange(
				other.m_Size,
				0);

		m_MipLevelCount =
			std::exchange(
				other.m_MipLevelCount,
				0);

		return *this;
	}

	void TextureCube::Bind(
		std::uint32_t textureUnit) const noexcept
	{
		glBindTextureUnit(
			textureUnit,
			m_Handle);
	}

	void TextureCube::AttachFaceToBoundFramebuffer(
		std::uint32_t faceIndex,
		std::uint32_t mipLevel) const
	{
		constexpr std::uint32_t faceCount = 6;

		if (faceIndex >= faceCount)
		{
			throw std::out_of_range(
				"Cubemap face index is out of range.");
		}

		if (mipLevel >= m_MipLevelCount)
		{
			throw std::out_of_range(
				"Cubemap mip level is out of range.");
		}

		glFramebufferTexture2D(
			GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X +
			static_cast<GLenum>(faceIndex),
			m_Handle,
			static_cast<GLint>(mipLevel));
	}

	void TextureCube::GenerateMipmaps()
		const noexcept
	{
		glGenerateTextureMipmap(
			m_Handle);
	}

	std::uint32_t TextureCube::Size()
		const noexcept
	{
		return m_Size;
	}

	std::uint32_t TextureCube::MipLevelCount()
		const noexcept
	{
		return m_MipLevelCount;
	}
}