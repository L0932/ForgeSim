#pragma once

#include <glm/vec2.hpp>

#include <ForgeSim/Platform/GlfwWindow.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneViewportState
    {
        glm::vec2 position{ 0.0f };
        glm::vec2 size{ 0.0f };
        glm::vec2 cursorPixels{ 0.0f };

        Platform::FramebufferExtent framebufferExtent{};

        bool hovered = false;

        [[nodiscard]] bool IsDrawable() const noexcept
        {
            return framebufferExtent.IsDrawable();
        }
    };

    class SceneViewportPanel final
    {
    public:
        // Begin and End must be paired, including when not drawable.
        [[nodiscard]] const SceneViewportState& Begin(
            const glm::vec2& framebufferScale);

        void DrawImage(unsigned int textureHandle);
        void End();

    private:
        SceneViewportState m_State;
    };
}