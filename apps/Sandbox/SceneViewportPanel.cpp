#include "SceneViewportPanel.hpp"
#include "SceneKeyboardShortcuts.hpp"

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

        if(m_Maximized)
        {
            // Keep the docked Scene alive behind a temporary, unsaved window.
            // Maximizing must not rewrite the user's dock layout.
            ImGui::Begin("Scene",nullptr,ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoCollapse);
            ImGui::End();
            const auto* viewport=ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
        }
        else
        {
            ImGui::SetNextWindowSize(
                ImVec2{ 800.0f, 600.0f },
                ImGuiCond_FirstUseEver);

            const float maximumSize =
                std::numeric_limits<float>::max();

            ImGui::SetNextWindowSizeConstraints(
                ImVec2{ 240.0f, 160.0f },
                ImVec2{ maximumSize, maximumSize });
        }
        if(m_FocusRequested) { ImGui::SetNextWindowFocus();m_FocusRequested=false; }

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2{ 0.0f, 0.0f });

        const bool visible = ImGui::Begin(
            m_Maximized ? "Scene (maximized)###ForgeSimMaximizedScene" : "Scene",
            nullptr,
            (m_Maximized ? ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoSavedSettings|
                ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize : 0) |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse);

        ImGui::PopStyleVar();

        if (!visible)
        {
            return m_State;
        }

        m_ToggleMaximizeAfterFrame=SceneShortcutPressed(ImGuiKey_Space,false,false,true);
        m_State.toggleSelectionHighlightRequested = ToggleSelectionHighlightShortcutPressed();
        m_State.deleteSelectionRequested = DeleteSelectionShortcutPressed();
        m_State.duplicateSelectionRequested = DuplicateSelectionShortcutPressed();

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

        // F, Ctrl+F and Esc opt into hover routing; other shortcuts retain keyboard-focus
        // requirements. Hover must be over scene content, not its tab/title bar.
        m_State.focusSelectionRequested = FocusSelectionShortcutPressed(false, m_State.hovered);
        m_State.orbitSelectionRequested = OrbitSelectionShortcutPressed(false, m_State.hovered);
        const bool escapeRequested = SceneShortcutPressed(ImGuiKey_Escape, false, false, false, m_State.hovered);
        // Esc only restores the panel layout; orbit is toggled with Ctrl+F.
        const bool restoreMaximized = m_Maximized && escapeRequested;
        m_ToggleMaximizeAfterFrame = m_ToggleMaximizeAfterFrame || restoreMaximized;

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
        if(m_ToggleMaximizeAfterFrame) { SetMaximized(!m_Maximized);m_ToggleMaximizeAfterFrame=false; }
    }
}
