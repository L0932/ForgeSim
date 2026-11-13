#pragma once

#include "EditorShortcutContext.hpp"
#include <imgui.h>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline bool SceneShortcutPressed(ImGuiKey key, bool editingBlocked = false,
        bool requireCtrl = false, bool requireShift = false)
    {
        const auto& io = ImGui::GetIO();
        return AllowSceneShortcut({
            .windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows),
            .editingBlocked = editingBlocked,
            .textInput = io.WantTextInput,
            .itemActive = ImGui::IsAnyItemActive(),
            .popupOpen = ImGui::IsPopupOpen(nullptr,
                ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel),
            .modifierHeld = (requireCtrl ? !io.KeyCtrl : io.KeyCtrl) || io.KeyAlt ||
                (requireShift ? !io.KeyShift : io.KeyShift) || io.KeySuper
        }) && ImGui::IsKeyPressed(key, false);
    }
    [[nodiscard]] inline bool DuplicateSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_D, blocked, true); }
    [[nodiscard]] inline bool ToggleSelectionHighlightShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_H, blocked, false, true); }
    [[nodiscard]] inline bool OrbitSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_F, blocked, true); }
    [[nodiscard]] inline bool ExitOrbitShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_Escape, blocked); }
    [[nodiscard]] inline bool FocusSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_F, blocked); }
    [[nodiscard]] inline bool DeleteSelectionShortcutPressed(bool blocked = false)
    { return SceneShortcutPressed(ImGuiKey_Delete, blocked); }
}
