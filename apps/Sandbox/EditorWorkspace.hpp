#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "EditorTheme.hpp"

namespace ForgeSim::Sandbox
{
    class EditorWorkspace final
    {
    public:
        // Call after creating the ImGui context, before the first frame.
        void Initialize();

        // Returns true when the default layout was applied.
        [[nodiscard]] bool Draw(const std::function<void()>& drawMainToolbar = {});

    private:
        void BuildDefaultLayout(unsigned int dockspaceId);

        // ImGui retains a pointer to this string.
        std::string m_SettingsPath;
        std::filesystem::path m_ThemePath;
        std::filesystem::path m_ViewPath;
        bool m_ShowMainToolbar = true;
        EditorTheme m_Theme = EditorTheme::ForgeSimDark;
        std::optional<EditorTheme> m_PendingTheme;
        bool m_ShowStyleEditor = false;
    };
}