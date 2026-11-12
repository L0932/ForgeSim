#pragma once

#include "SceneFilePanel.hpp"

#include <array>
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

namespace ForgeSim::Sandbox
{
    struct EditorMainToolbarResult
    {
        SceneFileAction fileAction = SceneFileAction::None;
        bool undo = false;
        bool redo = false;
    };

    // Reserve space above the dock workspace, below the menu bar.
    inline EditorMainToolbarResult DrawEditorMainToolbar(const SceneToolbarContext& context)
    {
        EditorMainToolbarResult result;
        const auto& style = ImGui::GetStyle();
        const float padding = style.FramePadding.x;
        const float height = ImGui::GetFrameHeight() + 2.0f * style.FramePadding.y;
        const auto flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, style.FramePadding.y));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_MenuBarBg]);
        if (ImGui::BeginViewportSideBar("##EditorMainToolbar", ImGui::GetMainViewport(),
            ImGuiDir_Up, height, flags))
        {
            const bool allowed = context.actionsAllowed && !ImGui::IsPopupOpen(nullptr,
                ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
            const std::array<const char*, 4> labels{"Open", "Save", "Undo", "Redo"};
            float needed = 3.0f * style.ItemSpacing.x + 2.0f * padding;
            for (const auto* label : labels)
                needed += ImGui::CalcTextSize(label).x + 2.0f * style.FramePadding.x;
            const bool compact = ImGui::GetContentRegionAvail().x < needed;

            if (compact)
            {
                if (ImGui::Button("Actions")) ImGui::OpenPopup("ToolbarActions");
                if (ImGui::BeginPopup("ToolbarActions"))
                {
                    // The popup itself is expected; only the caller's edit lock applies.
                    ImGui::BeginDisabled(!context.actionsAllowed);
                    if (ImGui::MenuItem("Open Scene...")) result.fileAction = SceneFileAction::Open;
                    if (ImGui::MenuItem("Import Model...")) result.fileAction = SceneFileAction::ImportModel;
                    if (ImGui::MenuItem("Save", "Ctrl+S")) result.fileAction =
                        context.hasScenePath ? SceneFileAction::Save : SceneFileAction::SaveAs;
                    ImGui::Separator();
                    result.undo = ImGui::MenuItem("Undo transform / pivot", "Ctrl+Z", false, context.canUndo);
                    result.redo = ImGui::MenuItem("Redo transform / pivot", "Ctrl+Y", false, context.canRedo);
                    ImGui::EndDisabled();
                    ImGui::EndPopup();
                }
            }
            else
            {
                ImGui::BeginDisabled(!allowed);
                if (ImGui::Button("Open")) result.fileAction = SceneFileAction::Open;
                ImGui::SetItemTooltip("Open a scene file");
                ImGui::SameLine();
                if (ImGui::Button("Save")) result.fileAction =
                    context.hasScenePath ? SceneFileAction::Save : SceneFileAction::SaveAs;
                ImGui::SetItemTooltip("Save scene (Ctrl+S)");
                ImGui::SameLine();
                ImGui::BeginDisabled(!context.canUndo);
                result.undo = ImGui::Button("Undo");
                ImGui::SetItemTooltip("Undo transform / pivot edit (Ctrl+Z)");
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled(!context.canRedo);
                result.redo = ImGui::Button("Redo");
                ImGui::SetItemTooltip("Redo transform / pivot edit (Ctrl+Y or Ctrl+Shift+Z)");
                ImGui::EndDisabled();
                ImGui::EndDisabled();
            }

            const std::string title = std::string(context.sceneName) + (context.dirty ? " *" : "");
            ImGui::SameLine();
            const float remaining = ImGui::GetContentRegionAvail().x;
            if (remaining > ImGui::GetFontSize() * 8.0f)
            {
                ImGui::Dummy({remaining, ImGui::GetFrameHeight()});
                auto min = ImGui::GetItemRectMin();
                const auto max = ImGui::GetItemRectMax();
                min.y += style.FramePadding.y;
                ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), min, max, max.x,
                    title.c_str(), nullptr, nullptr);
                ImGui::SetItemTooltip("%s", title.c_str());
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        return result;
    }
}
