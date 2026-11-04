#include <ForgeSim/Renderer/OpenGL/ShadowMap.hpp>

#include <stdexcept>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
    ShadowMap::ShadowMap(std::uint32_t resolution, Contents contents)
    {
        if (resolution == 0)
        {
            throw std::invalid_argument(
                "Shadow-map resolution must be positive.");
        }

        GLint maximumTextureSize = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumTextureSize);

        if (maximumTextureSize <= 0 ||
            resolution >
            static_cast<std::uint32_t>(maximumTextureSize))
        {
            throw std::invalid_argument(
                "Shadow-map resolution exceeds the supported texture size.");
        }

        try
        {
            glCreateFramebuffers(1, &m_Framebuffer);

            if (m_Framebuffer == 0)
            {
                throw std::runtime_error(
                    "Failed to create the shadow framebuffer.");
            }

            glCreateTextures(
                GL_TEXTURE_2D,
                1,
                &m_DepthTexture);

            if (m_DepthTexture == 0)
            {
                throw std::runtime_error(
                    "Failed to create the shadow depth texture.");
            }

            const auto size = static_cast<GLsizei>(resolution);

            glTextureStorage2D(
                m_DepthTexture,
                1,
                GL_DEPTH_COMPONENT32F,
                size,
                size);

            // Read individual depth samples. Filtering and comparison
            // will be implemented explicitly in the PBR shader.
            glTextureParameteri(
                m_DepthTexture,
                GL_TEXTURE_MIN_FILTER,
                GL_NEAREST);

            glTextureParameteri(
                m_DepthTexture,
                GL_TEXTURE_MAG_FILTER,
                GL_NEAREST);

            glTextureParameteri(
                m_DepthTexture,
                GL_TEXTURE_COMPARE_MODE,
                GL_NONE);

            glTextureParameteri(
                m_DepthTexture,
                GL_TEXTURE_WRAP_S,
                GL_CLAMP_TO_BORDER);

            glTextureParameteri(
                m_DepthTexture,
                GL_TEXTURE_WRAP_T,
                GL_CLAMP_TO_BORDER);

            // Samples beyond the texture edges represent far depth.
            constexpr GLfloat borderColor[] =
            {
                1.0F, 1.0F, 1.0F, 1.0F
            };

            glTextureParameterfv(
                m_DepthTexture,
                GL_TEXTURE_BORDER_COLOR,
                borderColor);

            glNamedFramebufferTexture(
                m_Framebuffer,
                GL_DEPTH_ATTACHMENT,
                m_DepthTexture,
                0);

            if (contents == Contents::Transmission)
            {
                glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorTexture);
                if (m_ColorTexture == 0)
                    throw std::runtime_error("Failed to create the transmission texture.");

                // RGB stores linear transmission; alpha stores full-precision depth.
                glTextureStorage2D(m_ColorTexture, 1, GL_RGBA32F, size, size);
                glTextureParameteri(m_ColorTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTextureParameteri(m_ColorTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTextureParameteri(m_ColorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(m_ColorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTextureParameterfv(m_ColorTexture, GL_TEXTURE_BORDER_COLOR, borderColor);
                glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_ColorTexture, 0);
            }

            glNamedFramebufferDrawBuffer(m_Framebuffer,
                m_ColorTexture != 0 ? GL_COLOR_ATTACHMENT0 : GL_NONE);

            glNamedFramebufferReadBuffer(
                m_Framebuffer,
                GL_NONE);

            const GLenum status =
                glCheckNamedFramebufferStatus(
                    m_Framebuffer,
                    GL_FRAMEBUFFER);

            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                throw std::runtime_error(
                    "Shadow framebuffer is incomplete.");
            }

            m_Resolution = resolution;
        }
        catch (...)
        {
            Destroy();
            throw;
        }
    }

    ShadowMap::~ShadowMap()
    {
        Destroy();
    }

    void ShadowMap::Bind() const noexcept
    {
        glBindFramebuffer(
            GL_FRAMEBUFFER,
            m_Framebuffer);
    }

    void ShadowMap::BindDepthTexture(
        std::uint32_t textureUnit) const noexcept
    {
        glBindTextureUnit(
            textureUnit,
            m_DepthTexture);
    }

    void ShadowMap::BindColorTexture(std::uint32_t textureUnit) const noexcept
    {
        glBindTextureUnit(textureUnit, m_ColorTexture);
    }

    std::uint32_t ShadowMap::Resolution() const noexcept
    {
        return m_Resolution;
    }

    void ShadowMap::Destroy() noexcept
    {
        if (m_Framebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_Framebuffer);
            m_Framebuffer = 0;
        }

        if (m_DepthTexture != 0)
        {
            glDeleteTextures(1, &m_DepthTexture);
            m_DepthTexture = 0;
        }

        if (m_ColorTexture != 0)
        {
            glDeleteTextures(1, &m_ColorTexture);
            m_ColorTexture = 0;
        }

        m_Resolution = 0;
    }
}