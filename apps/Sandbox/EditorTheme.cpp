#include "EditorTheme.hpp"

#include <fstream>
#include <string>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <ForgeSim/Core/Log.hpp>

namespace ForgeSim::Sandbox
{
    std::string_view EditorThemeName(EditorTheme theme) noexcept
    {
        switch (theme)
        {
        case EditorTheme::Light: return "Light";
        case EditorTheme::ImGuiClassic: return "ImGui Classic";
        case EditorTheme::ImGuiDark: return "ImGui Dark";
        default: return "ForgeSim Dark";
        }
    }

    EditorTheme LoadEditorTheme(const std::filesystem::path& path) noexcept
    {
        try
        {
            std::ifstream input(path);
            if (input)
            {
                const auto json = nlohmann::json::parse(input, nullptr, false);
                if (json.is_object() && json.value("version", 0) == 1)
                {
                    const auto name = json.value("theme", std::string{});
                    if (name == "Light") return EditorTheme::Light;
                    if (name == "ImGui Classic") return EditorTheme::ImGuiClassic;
                    if (name == "ImGui Dark") return EditorTheme::ImGuiDark;
                }
            }
        }
        catch (...)
        {
            // A local preference cannot prevent the editor from opening.
        }
        return EditorTheme::ForgeSimDark;
    }

    bool SaveEditorTheme(const std::filesystem::path& path, EditorTheme theme) noexcept
    {
        try
        {
            // This file stores only this preference. Scene and docking state
            // are owned by their existing separate files.
            std::ofstream output(path, std::ios::trunc);
            if (!output)
                return false;
            output << nlohmann::json{
                {"version", 1}, {"theme", EditorThemeName(theme)}
            }.dump(2) << '\n';
            output.close();
            return static_cast<bool>(output);
        }
        catch (...)
        {
            return false;
        }
    }

    void LoadEditorFont(const std::filesystem::path& fontPath)
    {
        // Called once after CreateContext(), before the first frame.
        std::error_code error;
        if (std::filesystem::is_regular_file(fontPath, error))
        {
            const auto utf8 = fontPath.generic_u8string();
            const std::string path{utf8.begin(), utf8.end()};
            auto& io = ImGui::GetIO();
            if (auto* font = io.Fonts->AddFontFromFileTTF(path.c_str(), 16.0f))
            {
                io.FontDefault = font;
                ImGui::GetStyle().FontSizeBase = 16.0f;
                return;
            }
        }
        // No font was added; ImGui supplies its embedded default.
        Core::LogWarning("Editor font unavailable; using the built-in ImGui font.");
    }

    void ApplyEditorTheme(EditorTheme theme)
    {
        // The built-in palette functions replace colors, leaving font/DPI
        // settings and widget dimensions untouched.
        switch (theme)
        {
        case EditorTheme::Light:
            ImGui::StyleColorsLight();
            return;
        case EditorTheme::ImGuiClassic:
            ImGui::StyleColorsClassic();
            return;
        case EditorTheme::ImGuiDark:
            ImGui::StyleColorsDark();
            return;
        default:
            ImGui::StyleColorsDark();
            break;
        }

        auto* colors = ImGui::GetStyle().Colors;
        const ImVec4 panel{0.115f, 0.126f, 0.144f, 1.0f};
        const ImVec4 raised{0.17f, 0.185f, 0.21f, 1.0f};
        const ImVec4 hovered{0.23f, 0.26f, 0.30f, 1.0f};
        const ImVec4 active{0.27f, 0.32f, 0.38f, 1.0f};
        const ImVec4 accent{0.34f, 0.58f, 0.79f, 1.0f};

        colors[ImGuiCol_Text] = {0.89f, 0.91f, 0.94f, 1.0f};
        colors[ImGuiCol_TextDisabled] = {0.53f, 0.57f, 0.63f, 1.0f};
        colors[ImGuiCol_WindowBg] = panel;
        colors[ImGuiCol_ChildBg] = {0.10f, 0.11f, 0.125f, 1.0f};
        colors[ImGuiCol_PopupBg] = {0.14f, 0.15f, 0.17f, 1.0f};
        colors[ImGuiCol_Border] = {0.25f, 0.27f, 0.30f, 1.0f};
        colors[ImGuiCol_FrameBg] = {0.08f, 0.09f, 0.105f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = hovered;
        colors[ImGuiCol_FrameBgActive] = active;
        colors[ImGuiCol_TitleBg] = panel;
        colors[ImGuiCol_TitleBgActive] = raised;
        colors[ImGuiCol_TitleBgCollapsed] = panel;
        colors[ImGuiCol_MenuBarBg] = {0.095f, 0.105f, 0.12f, 1.0f};
        colors[ImGuiCol_ScrollbarBg] = panel;
        colors[ImGuiCol_ScrollbarGrab] = raised;
        colors[ImGuiCol_ScrollbarGrabHovered] = hovered;
        colors[ImGuiCol_ScrollbarGrabActive] = active;
        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = {0.47f, 0.71f, 0.91f, 1.0f};
        colors[ImGuiCol_Button] = raised;
        colors[ImGuiCol_ButtonHovered] = hovered;
        colors[ImGuiCol_ButtonActive] = active;
        colors[ImGuiCol_Header] = {0.20f, 0.28f, 0.36f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = {0.25f, 0.35f, 0.45f, 1.0f};
        colors[ImGuiCol_HeaderActive] = {0.28f, 0.39f, 0.50f, 1.0f};
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = accent;
        colors[ImGuiCol_SeparatorActive] = accent;
        colors[ImGuiCol_ResizeGrip] = {0.34f, 0.58f, 0.79f, 0.18f};
        colors[ImGuiCol_ResizeGripHovered] = {0.34f, 0.58f, 0.79f, 0.60f};
        colors[ImGuiCol_ResizeGripActive] = accent;
        colors[ImGuiCol_Tab] = panel;
        colors[ImGuiCol_TabHovered] = hovered;
        colors[ImGuiCol_TabSelected] = raised;
        colors[ImGuiCol_TabSelectedOverline] = accent;
        colors[ImGuiCol_TabDimmed] = panel;
        colors[ImGuiCol_TabDimmedSelected] = raised;
        colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiCol_Border];
        colors[ImGuiCol_DockingPreview] = {0.34f, 0.58f, 0.79f, 0.50f};
        colors[ImGuiCol_DockingEmptyBg] = {0.07f, 0.08f, 0.09f, 1.0f};
        colors[ImGuiCol_TextSelectedBg] = {0.34f, 0.58f, 0.79f, 0.35f};
        colors[ImGuiCol_NavCursor] = accent;
    }
}
