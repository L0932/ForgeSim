#pragma once

#include <filesystem>
#include <string_view>

namespace ForgeSim::Sandbox
{
    enum class EditorTheme 
    { 
        ForgeSimDark, 
        Light, 
        ImGuiClassic,
        ImGuiDark,
        ForgeSimDarkBlue
    };

    [[nodiscard]] std::string_view EditorThemeName(EditorTheme theme) noexcept;
    [[nodiscard]] EditorTheme LoadEditorTheme(const std::filesystem::path& path) noexcept;
    [[nodiscard]] bool SaveEditorTheme(const std::filesystem::path& path, EditorTheme theme) noexcept;
    void LoadEditorFont(const std::filesystem::path& fontPath);
    void ApplyEditorTheme(EditorTheme theme);
}
