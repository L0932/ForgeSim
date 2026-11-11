#pragma once

#include <imgui.h>

namespace ForgeSim::Sandbox
{
    // ImGui shares Header colors between sections and selected rows.
    // Override only section rendering so selection keeps the theme's accent.
    inline bool EditorSectionHeader(const char* label, ImGuiTreeNodeFlags flags = 0)
    {
        const auto& style = ImGui::GetStyle();
        const auto& background = style.Colors[ImGuiCol_WindowBg];
        const auto& foreground = style.Colors[ImGuiCol_Text];
        const auto surface = [&](float contrast)
        {
            // Keep the palette's surface hue while separating headers from selection.
            return ImVec4(
                background.x + (foreground.x - background.x) * contrast,
                background.y + (foreground.y - background.y) * contrast,
                background.z + (foreground.z - background.z) * contrast, 1.0f);
        };
        ImGui::PushStyleColor(ImGuiCol_Header, surface(0.08f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, surface(0.14f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, surface(0.20f));
        const bool expanded = ImGui::CollapsingHeader(label, flags);
        ImGui::PopStyleColor(3);
        return expanded;
    }
}
