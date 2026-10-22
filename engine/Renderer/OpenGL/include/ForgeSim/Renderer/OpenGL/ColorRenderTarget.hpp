#pragma once

#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
    class ColorRenderTarget final
    {
    public:
        ColorRenderTarget(
            std::uint32_t width,
            std::uint32_t height);

        ~ColorRenderTarget();

        ColorRenderTarget(const ColorRenderTarget&) = delete;
        ColorRenderTarget& operator=(const ColorRenderTarget&) = delete;

        ColorRenderTarget(ColorRenderTarget&&) = delete;
        ColorRenderTarget& operator=(ColorRenderTarget&&) = delete;

        void Resize(
            std::uint32_t width,
            std::uint32_t height);

        void Bind() const noexcept;

        [[nodiscard]]
        unsigned int ColorTextureHandle() const noexcept;

        [[nodiscard]] std::uint32_t Width() const noexcept;
        [[nodiscard]] std::uint32_t Height() const noexcept;

    private:
        unsigned int m_Framebuffer = 0;
        unsigned int m_ColorTexture = 0;

        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;
    };
}