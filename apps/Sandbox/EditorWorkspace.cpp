#include "EditorWorkspace.hpp"
#include "SandboxPaths.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <ForgeSim/Core/Log.hpp>

namespace ForgeSim::Sandbox
{
    void EditorWorkspace::Initialize()
    {
        const std::filesystem::path settingsPath =
            EditorSettingsDirectory() / "imgui.ini";

        const std::u8string utf8Path =
            settingsPath.generic_u8string();

        m_SettingsPath.assign(
            utf8Path.begin(),
            utf8Path.end());

        ImGui::GetIO().IniFilename = m_SettingsPath.c_str();

        m_ThemePath = settingsPath.parent_path() / "editor-theme.json";
        m_ViewPath = settingsPath.parent_path() / "editor-view.json";
        try
        {
            std::ifstream input(m_ViewPath);
            const auto saved = nlohmann::json::parse(input, nullptr, false);
            if (saved.is_object() && saved.value("version", 0) == 1)
                m_ShowMainToolbar = saved.value("showMainToolbar", true);
        }
        catch (...) { /* Invalid local preferences keep the default view. */ }
        m_Theme = LoadEditorTheme(m_ThemePath);
        LoadEditorFont(BundledAssetDirectory() / "fonts" / "Karla-Regular.ttf");
        ApplyEditorTheme(m_Theme);

        // Set geometry once; palette changes leave these dimensions intact.
        auto& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;

        Core::LogInfo(
            "Editor layout settings: " + m_SettingsPath);
    }

    void EditorWorkspace::BuildDefaultLayout(
        unsigned int dockspaceId)
    {
        const ImGuiViewport* viewport =
            ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode(dockspaceId);

        ImGui::DockBuilderAddNode(
            dockspaceId,
            ImGuiDockNodeFlags_DockSpace);

        ImGui::DockBuilderSetNodePos(
            dockspaceId,
            viewport->WorkPos);

        ImGui::DockBuilderSetNodeSize(
            dockspaceId,
            viewport->WorkSize);

        ImGuiID center = dockspaceId;
        ImGuiID console = 0;
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.22f, &console, &center);
        ImGuiID left = 0;
        ImGuiID right = 0;

        ImGui::DockBuilderSplitNode(
            center,
            ImGuiDir_Left,
            0.23f,
            &left,
            &center);

        ImGui::DockBuilderSplitNode(
            center,
            ImGuiDir_Right,
            0.28f,
            &right,
            &center);

        ImGuiID leftTop = 0;
        ImGuiID leftBottom = 0;

        ImGui::DockBuilderSplitNode(
            left,
            ImGuiDir_Down,
            0.48f,
            &leftBottom,
            &leftTop);

        ImGui::DockBuilderDockWindow("Scene", center);
        ImGui::DockBuilderDockWindow("Hierarchy", leftTop);
        ImGui::DockBuilderDockWindow("Console###Scene File", console);
        ImGui::DockBuilderDockWindow("ForgeSim Debug", leftBottom);
        ImGui::DockBuilderDockWindow("Inspector", right);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    bool EditorWorkspace::Draw(const std::function<void()>& drawMainToolbar)
    {
        // Apply outside an open menu/window so its layout uses one palette
        // for the entire frame. Changes take effect on the following frame.
        if (m_PendingTheme)
        {
            m_Theme = *m_PendingTheme;
            m_PendingTheme.reset();
            ApplyEditorTheme(m_Theme);
            if (!SaveEditorTheme(m_ThemePath, m_Theme))
                Core::LogWarning("Could not save editor theme preference.");
        }

        bool resetRequested = false;

        // Append View alongside the existing File menu.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Show Main Toolbar", nullptr, &m_ShowMainToolbar))
                {
                    try
                    {
                        std::ofstream output(m_ViewPath, std::ios::trunc);
                        output << nlohmann::json{{"version", 1},
                            {"showMainToolbar", m_ShowMainToolbar}}.dump(2) << '\n';
                        output.close();
                        if (!output) Core::LogWarning("Could not save editor view preference.");
                    }
                    catch (...) { Core::LogWarning("Could not save editor view preference."); }
                }
                ImGui::Separator();
                resetRequested =
                    ImGui::MenuItem("Reset Layout");

                if (ImGui::BeginMenu("Theme"))
                {
                    for (const auto theme : {EditorTheme::ForgeSimDark,
                        EditorTheme::ForgeSimDarkBlue, EditorTheme::Light,
                        EditorTheme::ImGuiClassic, EditorTheme::ImGuiDark})
                    {
                        if (ImGui::MenuItem(EditorThemeName(theme).data(), nullptr, m_Theme == theme))
                            m_PendingTheme = theme;
                    }
                    ImGui::EndMenu();
                }

                ImGui::MenuItem("Style Editor", nullptr, &m_ShowStyleEditor);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Reserve toolbar space after menu submission and before the dockspace.
        if (m_ShowMainToolbar && drawMainToolbar)
            drawMainToolbar();

        if (m_ShowStyleEditor)
        {
            if (ImGui::Begin("Style Editor", &m_ShowStyleEditor))
            {
                ImGui::ShowStyleEditor();
            }

            ImGui::End();
        }

        const ImGuiID dockspaceId =
            ImGui::GetID("ForgeSimWorkspace");

        const bool applyDefault =
            resetRequested ||
            ImGui::DockBuilderGetNode(dockspaceId) == nullptr;

        if (applyDefault)
        {
            BuildDefaultLayout(dockspaceId);
        }

        ImGui::DockSpaceOverViewport(
            dockspaceId,
            ImGui::GetMainViewport());

        return applyDefault;
    }
}