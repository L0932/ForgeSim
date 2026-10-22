#include <ForgeSim/Renderer/OpenGL/ColorRenderTarget.hpp>

#include <stdexcept>
#include <utility>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
    ColorRenderTarget::ColorRenderTarget(
        std::uint32_t width,
        std::uint32_t height)
    {
        Resize(width, height);
    }

    ColorRenderTarget::~ColorRenderTarget()
    {
        glDeleteFramebuffers(1, &m_Framebuffer);
        glDeleteTextures(1, &m_ColorTexture);
    }

    void ColorRenderTarget::Resize(
        std::uint32_t width,
        std::uint32_t height)
    {
        if (width == 0 || height == 0)
        {
            throw std::invalid_argument(
                "Color render-target dimensions must be positive.");
        }

        if (width == m_Width && height == m_Height)
        {
            return;
        }

        GLint maximumSize = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumSize);

        if (maximumSize <= 0 ||
            width > static_cast<std::uint32_t>(maximumSize) ||
            height > static_cast<std::uint32_t>(maximumSize))
        {
            throw std::invalid_argument(
                "Color render-target dimensions exceed the GPU limit.");
        }

        GLuint replacementFramebuffer = 0;
        GLuint replacementTexture = 0;

        try
        {
            glCreateFramebuffers(1, &replacementFramebuffer);
            glCreateTextures(
                GL_TEXTURE_2D,
                1,
                &replacementTexture);

            if (replacementFramebuffer == 0 ||
                replacementTexture == 0)
            {
                throw std::runtime_error(
                    "Failed to create the color render target.");
            }

            glTextureStorage2D(
                replacementTexture,
                1,
                GL_RGBA8,
                static_cast<GLsizei>(width),
                static_cast<GLsizei>(height));

            glTextureParameteri(
                replacementTexture,
                GL_TEXTURE_MIN_FILTER,
                GL_LINEAR);

            glTextureParameteri(
                replacementTexture,
                GL_TEXTURE_MAG_FILTER,
                GL_LINEAR);

            glTextureParameteri(
                replacementTexture,
                GL_TEXTURE_WRAP_S,
                GL_CLAMP_TO_EDGE);

            glTextureParameteri(
                replacementTexture,
                GL_TEXTURE_WRAP_T,
                GL_CLAMP_TO_EDGE);

            glNamedFramebufferTexture(
                replacementFramebuffer,
                GL_COLOR_ATTACHMENT0,
                replacementTexture,
                0);

            glNamedFramebufferDrawBuffer(
                replacementFramebuffer,
                GL_COLOR_ATTACHMENT0);

            if (glCheckNamedFramebufferStatus(
                replacementFramebuffer,
                GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                throw std::runtime_error(
                    "Color framebuffer is incomplete.");
            }
        }
        catch (...)
        {
            glDeleteFramebuffers(1, &replacementFramebuffer);
            glDeleteTextures(1, &replacementTexture);
            throw;
        }

        // Preserve the previous target if creating its replacement fails.
        std::swap(m_Framebuffer, replacementFramebuffer);
        std::swap(m_ColorTexture, replacementTexture);

        m_Width = width;
        m_Height = height;

        glDeleteFramebuffers(1, &replacementFramebuffer);
        glDeleteTextures(1, &replacementTexture);
    }

    void ColorRenderTarget::Bind() const noexcept
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    }

    unsigned int ColorRenderTarget::ColorTextureHandle() const noexcept
    {
        return m_ColorTexture;
    }

    std::uint32_t ColorRenderTarget::Width() const noexcept
    {
        return m_Width;
    }

    std::uint32_t ColorRenderTarget::Height() const noexcept
    {
        return m_Height;
    }
}