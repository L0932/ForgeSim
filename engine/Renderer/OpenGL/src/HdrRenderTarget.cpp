#include <ForgeSim/Renderer/OpenGL/HdrRenderTarget.hpp>

#include <stdexcept>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	HdrRenderTarget::HdrRenderTarget(
		std::uint32_t width,
		std::uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			throw std::invalid_argument(
				"HDR render-target dimensions must be positive.");
		}

		glCreateFramebuffers(
			1,
			&m_Framebuffer);

		if (m_Framebuffer == 0)
		{
			throw std::runtime_error(
				"Failed to create the HDR framebuffer.");
		}

		try
		{
			CreateAttachments(width, height);
		}
		catch (...)
		{
			glDeleteFramebuffers(
				1,
				&m_Framebuffer);

			m_Framebuffer = 0;
			throw;
		}
	}

	HdrRenderTarget::~HdrRenderTarget()
	{
		DestroyAttachments();

		if (m_Framebuffer != 0)
		{
			glDeleteFramebuffers(
				1,
				&m_Framebuffer);
		}
	}

	void HdrRenderTarget::Resize(
		std::uint32_t width,
		std::uint32_t height)
	{
		// A minimized window does not provide a drawable target.
		if (width == 0 || height == 0)
		{
			return;
		}

		if (width == m_Width &&
			height == m_Height)
		{
			return;
		}

		DestroyAttachments();
		CreateAttachments(width, height);
	}

	void HdrRenderTarget::Bind() const noexcept
	{
		glBindFramebuffer(
			GL_FRAMEBUFFER,
			m_Framebuffer);
	}

	void HdrRenderTarget::BindDefault() noexcept
	{
		glBindFramebuffer(
			GL_FRAMEBUFFER,
			0);
	}

	void HdrRenderTarget::BindColorTexture(
		std::uint32_t textureUnit) const noexcept
	{
		glBindTextureUnit(
			textureUnit,
			m_ColorTexture);
	}

	std::uint32_t HdrRenderTarget::Width()
		const noexcept
	{
		return m_Width;
	}

	std::uint32_t HdrRenderTarget::Height()
		const noexcept
	{
		return m_Height;
	}

	void HdrRenderTarget::CreateAttachments(
		std::uint32_t width,
		std::uint32_t height)
	{
		const auto glWidth =
			static_cast<GLsizei>(width);

		const auto glHeight =
			static_cast<GLsizei>(height);

		glCreateTextures(
			GL_TEXTURE_2D,
			1,
			&m_ColorTexture);

		glTextureStorage2D(
			m_ColorTexture,
			1,
			GL_RGBA16F,
			glWidth,
			glHeight);

		glTextureParameteri(
			m_ColorTexture,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);

		glTextureParameteri(
			m_ColorTexture,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);

		glTextureParameteri(
			m_ColorTexture,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);

		glTextureParameteri(
			m_ColorTexture,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);

		glNamedFramebufferTexture(
			m_Framebuffer,
			GL_COLOR_ATTACHMENT0,
			m_ColorTexture,
			0);

		glCreateRenderbuffers(
			1,
			&m_DepthRenderbuffer);

		glNamedRenderbufferStorage(
			m_DepthRenderbuffer,
			GL_DEPTH_COMPONENT24,
			glWidth,
			glHeight);

		glNamedFramebufferRenderbuffer(
			m_Framebuffer,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			m_DepthRenderbuffer);

		const GLenum framebufferStatus =
			glCheckNamedFramebufferStatus(
				m_Framebuffer,
				GL_FRAMEBUFFER);

		if (framebufferStatus !=
			GL_FRAMEBUFFER_COMPLETE)
		{
			DestroyAttachments();

			throw std::runtime_error(
				"HDR framebuffer is incomplete.");
		}

		m_Width = width;
		m_Height = height;
	}

	void HdrRenderTarget::DestroyAttachments()
		noexcept
	{
		if (m_DepthRenderbuffer != 0)
		{
			glDeleteRenderbuffers(
				1,
				&m_DepthRenderbuffer);

			m_DepthRenderbuffer = 0;
		}

		if (m_ColorTexture != 0)
		{
			glDeleteTextures(
				1,
				&m_ColorTexture);

			m_ColorTexture = 0;
		}

		m_Width = 0;
		m_Height = 0;
	}
}