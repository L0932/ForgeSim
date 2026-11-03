#include <ForgeSim/Renderer/OpenGL/FxaaRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/ColorRenderTarget.hpp>

#include <array>
#include <stdexcept>
#include <utility>
#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
    namespace
    {
        constexpr std::array<std::string_view, 6> uniforms{
            "uSourceTexture", "uTexelSize", "uEnabled",
            "uRelativeContrastThreshold", "uMinimumContrastThreshold", "uSubpixelBlending"
        };

        struct PassState
        {
            static constexpr std::array<GLenum, 9> caps{
                GL_DEPTH_TEST, GL_BLEND, GL_CULL_FACE, GL_SCISSOR_TEST,
                GL_FRAMEBUFFER_SRGB, GL_STENCIL_TEST, GL_RASTERIZER_DISCARD,
                GL_COLOR_LOGIC_OP, GL_DITHER
            };
            std::array<GLboolean, caps.size()> enabled{};
            std::array<GLint, 4> viewport{};
            std::array<GLboolean, 4> mask{};
            std::array<GLint, 2> polygon{};
            GLint draw{}, read{}, program{}, vao{}, active{}, texture{}, sampler{};

            PassState()
            {
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
                glGetIntegerv(GL_VIEWPORT, viewport.data());
                glGetIntegerv(GL_CURRENT_PROGRAM, &program);
                glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
                glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
                glActiveTexture(GL_TEXTURE0);
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
                glGetIntegeri_v(GL_SAMPLER_BINDING, 0, &sampler);
                glGetBooleanv(GL_COLOR_WRITEMASK, mask.data());
                glGetIntegerv(GL_POLYGON_MODE, polygon.data());
                for (std::size_t i = 0; i < caps.size(); ++i)
                {
                    enabled[i] = glIsEnabled(caps[i]);
                    glDisable(caps[i]);
                }
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            ~PassState()
            {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
                glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
                glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                glUseProgram(static_cast<GLuint>(program));
                glBindVertexArray(static_cast<GLuint>(vao));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
                glBindSampler(0, static_cast<GLuint>(sampler));
                glActiveTexture(static_cast<GLenum>(active));
                glColorMask(mask[0], mask[1], mask[2], mask[3]);
                glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(polygon[0]));
                for (std::size_t i = 0; i < caps.size(); ++i)
                {
                    if (enabled[i]) glEnable(caps[i]);
                    else glDisable(caps[i]);
                }
            }
            PassState(const PassState&) = delete;
            PassState& operator=(const PassState&) = delete;
        };
    }

    FxaaRenderer::FxaaRenderer(std::string_view vertex, std::string_view fragment)
        : m_Program(vertex, fragment, uniforms)
        , m_Sampler(SamplerSpecification{
            .minificationFilter = MinificationFilter::Linear,
            .magnificationFilter = MagnificationFilter::Linear,
            .wrapU = SamplerWrap::ClampToEdge,
            .wrapV = SamplerWrap::ClampToEdge})
    {}

    void FxaaRenderer::RenderToTarget(const ColorRenderTarget& source,
        ColorRenderTarget& destination, const FxaaSettings& settings)
    {
        ValidateFxaaSettings(settings);
        if (&source == &destination)
            throw std::invalid_argument("FXAA requires separate source and destination textures.");
        const PassState state;
        destination.Resize(source.Width(), source.Height());
        destination.Bind();
        Draw(source, settings);
    }

    void FxaaRenderer::RenderToDefaultFramebuffer(const ColorRenderTarget& source,
        const FxaaSettings& settings)
    {
        ValidateFxaaSettings(settings);
        const PassState state;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        Draw(source, settings);
    }

    void FxaaRenderer::Draw(const ColorRenderTarget& source, const FxaaSettings& settings)
    {
        const auto step = CalculateFxaaTexelSize(source.Width(), source.Height());
        if (!step) return;

        glViewport(0, 0, static_cast<GLsizei>(source.Width()), static_cast<GLsizei>(source.Height()));
        glBindTexture(GL_TEXTURE_2D, source.ColorTextureHandle());
        m_Sampler.Bind(0);
        m_Program.Bind();
        m_Program.SetInt(uniforms[0], 0);
        m_Program.SetVector4(uniforms[1], glm::vec4{(*step)[0], (*step)[1], 0.0f, 0.0f});
        m_Program.SetInt(uniforms[2], settings.enabled ? 1 : 0);
        m_Program.SetFloat(uniforms[3], settings.relativeContrastThreshold);
        m_Program.SetFloat(uniforms[4], settings.minimumContrastThreshold);
        m_Program.SetFloat(uniforms[5], settings.subpixelBlending);
        m_VertexArray.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void FxaaRenderer::ReloadShaders(std::string_view vertex, std::string_view fragment)
    {
        ShaderProgram replacement{vertex, fragment, uniforms};
        m_Program = std::move(replacement);
    }
}
