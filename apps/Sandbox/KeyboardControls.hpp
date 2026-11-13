#pragma once

#include <array>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct KeyboardControl
    {
        std::string_view group;
        std::string_view shortcut;
        std::string_view action;
        std::string_view context;
    };

    inline constexpr auto KeyboardControls = std::to_array<KeyboardControl>({
        {"Camera", "RMB + mouse", "Look around", "Hold RMB over Scene to begin navigation"},
        {"Camera", "RMB + W / A / S / D", "Fly forward / left / backward / right", "Scene navigation"},
        {"Camera", "RMB + wheel", "Increase / decrease fly speed", "Scene navigation; saved with scene"},
        {"Camera", "Ctrl while flying", "Move at one tenth of fly speed", "Scene navigation"},
        {"Selection", "Left click", "Select an object or light", "Scene or Hierarchy; empty Scene space clears selection"},
        {"Selection", "F", "Focus selected object or light", "Scene or Hierarchy focused; navigation stopped"},
        {"Selection", "Delete", "Delete selected model or local light", "Scene or Hierarchy focused; main directional light is disabled"},
        {"Transforms", "W", "Move tool", "Navigation stopped; no text entry or active edit"},
        {"Transforms", "E", "Rotate tool", "Navigation stopped; no text entry or active edit"},
        {"Transforms", "R", "Scale tool", "Navigation stopped; no text entry or active edit"},
        {"Transforms", "X", "Toggle world / local axes", "Navigation stopped; no text entry or active edit"},
        {"History", "Ctrl+Z", "Undo transform, pivot, or deletion", "No text entry, active edit, or scene-change confirmation"},
        {"History", "Ctrl+Y / Ctrl+Shift+Z", "Redo transform, pivot, or deletion", "No text entry, active edit, or scene-change confirmation"},
        {"Files", "Ctrl+S", "Save scene / choose destination", "No text entry, active edit, navigation, or confirmation"},
        {"Panels", "F1", "Show / hide ForgeSim Debug", "No text entry or active control"},
        {"Console", "Enter", "Execute command", "Console input focused"},
        {"Console", "Up / Down", "Recall command history", "Console input focused"},
        {"Fields", "Ctrl+click slider", "Type a precise value", "Supported numeric sliders, including fly speed"},
        {"Fields", "Enter", "Apply clipping value", "Near clip or Far clip field focused"}
    });
}
