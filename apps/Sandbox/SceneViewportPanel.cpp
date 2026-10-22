#include "SceneViewportPanel.hpp"

#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <limits>

namespace ForgeSim::Sandbox
{
    const SceneViewportState& SceneViewportPanel::Begin(
        const glm::vec2& framebufferScale)
    {
        m_State = {};

        ImGui::SetNextWindowSize(
            ImVec2{ 800.0f, 600.0f },
            ImGuiCond_FirstUseEver);

        const float maximumSize =
            std::numeric_limits<float>::max();

        ImGui::SetNextWindowSizeConstraints(
            ImVec2{ 240.0f, 160.0f },
            ImVec2{ maximumSize, maximumSize });

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2{ 0.0f, 0.0f });

        const bool visible = ImGui::Begin(
            "Scene",
            nullptr,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse);

        ImGui::PopStyleVar();

        if (!visible)
        {
            return m_State;
        }

        const ImVec2 position = ImGui::GetCursorScreenPos();
        const ImVec2 size = ImGui::GetContentRegionAvail();

        if (size.x < 1.0f || size.y < 1.0f)
        {
            return m_State;
        }

        m_State.position = { position.x, position.y };
        m_State.size = { size.x, size.y };

        m_State.framebufferExtent = {
            std::max(
                1,
                static_cast<int>(
                    std::floor(size.x * framebufferScale.x))),
            std::max(
                1,
                static_cast<int>(
                    std::floor(size.y * framebufferScale.y)))
        };

        const ImVec2 mouse = ImGui::GetMousePos();

        m_State.cursorPixels = {
            (mouse.x - position.x) / size.x *
                static_cast<float>(
                    m_State.framebufferExtent.width),

            (mouse.y - position.y) / size.y *
                static_cast<float>(
                    m_State.framebufferExtent.height)
        };

        m_State.hovered =
            ImGui::IsWindowHovered() &&
            ImGui::IsMouseHoveringRect(
                position,
                ImVec2{
                    position.x + size.x,
                    position.y + size.y
                });

        return m_State;
    }

    void SceneViewportPanel::DrawImage(unsigned int textureHandle)
    {
        if (!m_State.IsDrawable() || textureHandle == 0)
        {
            return;
        }

        ImGui::Image(
            ImTextureRef{
                static_cast<ImTextureID>(textureHandle)
            },
            ImVec2{ m_State.size.x, m_State.size.y },
            ImVec2{ 0.0f, 1.0f },
            ImVec2{ 1.0f, 0.0f });
    }

    void SceneViewportPanel::End()
    {
        ImGui::End();
    }
}