#pragma once

namespace ForgeSim::Sandbox
{
    struct EditorShortcutContext
    {
        bool windowFocused = false;
        bool editingBlocked = false;
        bool textInput = false;
        bool itemActive = false;
        bool popupOpen = false;
        bool modifierHeld = false;
    };

    [[nodiscard]] constexpr bool AllowSceneShortcut(const EditorShortcutContext& context) noexcept
    {
        return context.windowFocused && !context.editingBlocked && !context.textInput &&
            !context.itemActive && !context.popupOpen && !context.modifierHeld;
    }
}
