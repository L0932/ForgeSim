#pragma once

#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
    // Requires a current OpenGL context during construction,
    // use, and destruction.
    class ShadowMap final
    {
    public:
        explicit ShadowMap(std::uint32_t resolution);
        ~ShadowMap();

        ShadowMap(const ShadowMap&) = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        ShadowMap(ShadowMap&&) = delete;
        ShadowMap& operator=(ShadowMap&&) = delete;

        // Binding does not change the viewport or other render state.
        void Bind() const noexcept;

        void BindDepthTexture(
            std::uint32_t textureUnit) const noexcept;

        [[nodiscard]]
        std::uint32_t Resolution() const noexcept;

    private:
        void Destroy() noexcept;

        unsigned int m_Framebuffer = 0;
        unsigned int m_DepthTexture = 0;
        std::uint32_t m_Resolution = 0;
    };
}