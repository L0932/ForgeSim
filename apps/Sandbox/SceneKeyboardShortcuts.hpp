#pragma once

#include "EditorShortcutContext.hpp"
#include <imgui.h>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline bool SceneShortcutPressed(ImGuiKey key, bool editingBlocked = false)
    {
        const auto& io = ImGui::GetIO();
        return AllowSceneShortcut({
            .windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows),
            .editingBlocked = editingBlocked,
            .textInput = io.WantTextInput,
            .itemActive = ImGui::IsAnyItemActive(),
            .popupOpen = ImGui::IsPopupOpen(nullptr,
                ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel),
            .modifierHeld = io.KeyCtrl || io.KeyAlt || io.KeyShift || io.KeySuper
        }) && ImGui::IsKeyPressed(key, false);
    }
    [[nodiscard]] inline bool FocusSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_F, blocked); }
    [[nodiscard]] inline bool DeleteSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_Delete, blocked); }
}
