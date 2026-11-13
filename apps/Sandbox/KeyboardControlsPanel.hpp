#pragma once

#include "KeyboardControls.hpp"
#include <imgui.h>
#include <string>

namespace ForgeSim::Sandbox
{
    inline void DrawKeyboardControls(bool& open, ImGuiTextFilter& filter)
    {
        if (!open) return;
        ImGui::SetNextWindowSize(ImVec2{760, 480}, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Keyboard Controls", &open))
        {
            filter.Draw("Search", -1.0f);
            ImGui::TextWrapped("F requires keyboard focus in Scene or Hierarchy. Text fields, active controls, "
                "popups, scene-change dialogs, and camera navigation suppress selection shortcuts.");
            if (ImGui::BeginTable("Controls", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, 170.0f);
                ImGui::TableSetupColumn("Action");
                ImGui::TableSetupColumn("Context");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
                for (const auto& control : KeyboardControls)
                {
                    // Filter one searchable record, including its category.
                    const auto text = std::string(control.group) + " " + std::string(control.shortcut) +
                        " " + std::string(control.action) + " " + std::string(control.context);
                    if (!filter.PassFilter(text.c_str())) continue;
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextWrapped("%s", control.shortcut.data());
                    ImGui::TableNextColumn(); ImGui::TextWrapped("%s", control.action.data());
                    ImGui::TableNextColumn(); ImGui::TextWrapped("%s", control.context.data());
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}
