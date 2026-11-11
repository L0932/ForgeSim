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
        case EditorTheme::ForgeSimDarkBlue: return "ForgeSim Dark Blue";
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
                    if (name == "ForgeSim Dark Blue") return EditorTheme::ForgeSimDarkBlue;
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
        {
            ImGui::StyleColorsLight();
            auto* colors = ImGui::GetStyle().Colors;
            // A soft gray workspace avoids large white surfaces beside the viewport.
            const ImVec4 panel{0.87f, 0.87f, 0.86f, 1.0f};
            const ImVec4 raised{0.74f, 0.75f, 0.75f, 1.0f};
            const ImVec4 hovered{0.67f, 0.73f, 0.78f, 1.0f};
            const ImVec4 active{0.40f, 0.61f, 0.80f, 1.0f};
            const ImVec4 accent{0.24f, 0.43f, 0.61f, 1.0f};
            colors[ImGuiCol_Text] = {0.08f, 0.10f, 0.13f, 1.0f};
            colors[ImGuiCol_TextDisabled] = {0.39f, 0.42f, 0.45f, 1.0f};
            colors[ImGuiCol_WindowBg] = panel;
            colors[ImGuiCol_ChildBg] = panel;
            colors[ImGuiCol_PopupBg] = {0.86f, 0.87f, 0.88f, 1.0f};
            colors[ImGuiCol_Border] = {0.43f, 0.46f, 0.49f, 1.0f};
            colors[ImGuiCol_FrameBg] = {0.96f, 0.96f, 0.95f, 1.0f};
            colors[ImGuiCol_FrameBgHovered] = {0.84f, 0.87f, 0.90f, 1.0f};
            colors[ImGuiCol_FrameBgActive] = {0.79f, 0.84f, 0.89f, 1.0f};
            colors[ImGuiCol_TitleBg] = raised;
            colors[ImGuiCol_TitleBgActive] = hovered;
            colors[ImGuiCol_TitleBgCollapsed] = raised;
            colors[ImGuiCol_MenuBarBg] = raised;
            colors[ImGuiCol_ScrollbarBg] = panel;
            colors[ImGuiCol_ScrollbarGrab] = {0.59f, 0.62f, 0.65f, 1.0f};
            colors[ImGuiCol_ScrollbarGrabHovered] = {0.49f, 0.53f, 0.57f, 1.0f};
            colors[ImGuiCol_ScrollbarGrabActive] = accent;
            colors[ImGuiCol_CheckMark] = accent;
            colors[ImGuiCol_SliderGrab] = accent;
            colors[ImGuiCol_SliderGrabActive] = {0.18f, 0.35f, 0.52f, 1.0f};
            colors[ImGuiCol_Button] = raised;
            colors[ImGuiCol_ButtonHovered] = hovered;
            colors[ImGuiCol_ButtonActive] = active;
            colors[ImGuiCol_Header] = {0.52f, 0.70f, 0.87f, 1.0f};
            colors[ImGuiCol_HeaderHovered] = {0.46f, 0.66f, 0.85f, 1.0f};
            colors[ImGuiCol_HeaderActive] = active;
            colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
            colors[ImGuiCol_SeparatorHovered] = accent;
            colors[ImGuiCol_SeparatorActive] = accent;
            colors[ImGuiCol_Tab] = raised;
            colors[ImGuiCol_TabHovered] = hovered;
            colors[ImGuiCol_TabSelected] = panel;
            colors[ImGuiCol_TabSelectedOverline] = accent;
            colors[ImGuiCol_TabDimmed] = raised;
            colors[ImGuiCol_TabDimmedSelected] = panel;
            colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiCol_Border];
            colors[ImGuiCol_DockingEmptyBg] = raised;
            colors[ImGuiCol_DockingPreview] = {accent.x, accent.y, accent.z, 0.40f};
            colors[ImGuiCol_ResizeGrip] = {accent.x, accent.y, accent.z, 0.20f};
            colors[ImGuiCol_ResizeGripHovered] = {accent.x, accent.y, accent.z, 0.55f};
            colors[ImGuiCol_ResizeGripActive] = accent;
            colors[ImGuiCol_TextSelectedBg] = {accent.x, accent.y, accent.z, 0.25f};
            colors[ImGuiCol_NavCursor] = accent;
            return;
        }
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

        if (theme == EditorTheme::ForgeSimDarkBlue)
        {
            // Match lopezdeveloper.net: navy surfaces and #4f94ff accents.
            auto* colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_Text] = {0.949020f, 0.964706f, 0.988235f, 1.00f};
            colors[ImGuiCol_TextDisabled] = {0.647059f, 0.705882f, 0.780392f, 1.00f};
            colors[ImGuiCol_WindowBg] = {0.050980f, 0.098039f, 0.156863f, 1.00f};
            colors[ImGuiCol_ChildBg] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_PopupBg] = {0.070588f, 0.133333f, 0.215686f, 1.00f};
            colors[ImGuiCol_Border] = {0.141176f, 0.211765f, 0.298039f, 1.00f};
            colors[ImGuiCol_FrameBg] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_FrameBgHovered] = {0.094118f, 0.180392f, 0.286275f, 1.00f};
            colors[ImGuiCol_FrameBgActive] = {0.125490f, 0.243137f, 0.376471f, 1.00f};
            colors[ImGuiCol_TitleBg] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_TitleBgActive] = {0.086275f, 0.164706f, 0.262745f, 1.00f};
            colors[ImGuiCol_TitleBgCollapsed] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_MenuBarBg] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_ScrollbarBg] = {0.050980f, 0.098039f, 0.156863f, 1.00f};
            colors[ImGuiCol_ScrollbarGrab] = {0.141176f, 0.211765f, 0.298039f, 1.00f};
            colors[ImGuiCol_ScrollbarGrabHovered] = {0.211765f, 0.333333f, 0.454902f, 1.00f};
            colors[ImGuiCol_ScrollbarGrabActive] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_CheckMark] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_SliderGrab] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_SliderGrabActive] = {0.662745f, 0.800000f, 1.000000f, 1.00f};
            colors[ImGuiCol_Button] = {0.094118f, 0.168627f, 0.262745f, 1.00f};
            colors[ImGuiCol_ButtonHovered] = {0.141176f, 0.247059f, 0.376471f, 1.00f};
            colors[ImGuiCol_ButtonActive] = {0.176471f, 0.337255f, 0.521569f, 1.00f};
            colors[ImGuiCol_Header] = {0.125490f, 0.247059f, 0.396078f, 1.00f};
            colors[ImGuiCol_HeaderHovered] = {0.164706f, 0.329412f, 0.525490f, 1.00f};
            colors[ImGuiCol_HeaderActive] = {0.196078f, 0.407843f, 0.658824f, 1.00f};
            colors[ImGuiCol_Separator] = {0.141176f, 0.211765f, 0.298039f, 1.00f};
            colors[ImGuiCol_SeparatorHovered] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_SeparatorActive] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_Tab] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_TabHovered] = {0.141176f, 0.247059f, 0.376471f, 1.00f};
            colors[ImGuiCol_TabSelected] = {0.086275f, 0.164706f, 0.262745f, 1.00f};
            colors[ImGuiCol_TabSelectedOverline] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_TabDimmed] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_TabDimmedSelected] = {0.070588f, 0.133333f, 0.215686f, 1.00f};
            colors[ImGuiCol_TabDimmedSelectedOverline] = {0.141176f, 0.211765f, 0.298039f, 1.00f};
            colors[ImGuiCol_DockingEmptyBg] = {0.027451f, 0.066667f, 0.113725f, 1.00f};
            colors[ImGuiCol_NavCursor] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_ResizeGrip] = {0.309804f, 0.580392f, 1.000000f, 0.18f};
            colors[ImGuiCol_ResizeGripHovered] = {0.309804f, 0.580392f, 1.000000f, 0.60f};
            colors[ImGuiCol_ResizeGripActive] = {0.309804f, 0.580392f, 1.000000f, 1.00f};
            colors[ImGuiCol_DockingPreview] = {0.309804f, 0.580392f, 1.000000f, 0.50f};
            colors[ImGuiCol_TextSelectedBg] = {0.309804f, 0.580392f, 1.000000f, 0.35f};
            return;
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
