#pragma once

#include <string_view>
#include <ForgeSim/Renderer/FxaaSettings.hpp>
#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
    class ColorRenderTarget;

    // Input and output must be separate textures containing display-encoded RGB.
    class FxaaRenderer final
    {
    public:
        FxaaRenderer(std::string_view vertex, std::string_view fragment);
        // Destination must not be bound on entry if it needs resizing.
        void RenderToTarget(const ColorRenderTarget& source, ColorRenderTarget& destination,
            const FxaaSettings& settings);
        void RenderToDefaultFramebuffer(const ColorRenderTarget& source, const FxaaSettings& settings);
        void ReloadShaders(std::string_view vertex, std::string_view fragment);

    private:
        void Draw(const ColorRenderTarget& source, const FxaaSettings& settings);
        ShaderProgram m_Program;
        Sampler m_Sampler;
        VertexArray m_VertexArray;
    };
}
