#include "EditorWorkspace.hpp"

#include <filesystem>

#include <imgui.h>
#include <imgui_internal.h>

#include <ForgeSim/Core/Log.hpp>

namespace ForgeSim::Sandbox
{
    void EditorWorkspace::Initialize()
    {
        const std::filesystem::path settingsPath =
            std::filesystem::absolute(
                std::filesystem::path{
                    FORGESIM_SANDBOX_ASSET_DIRECTORY
                }.parent_path() / "imgui.ini")
            .lexically_normal();

        const std::u8string utf8Path =
            settingsPath.generic_u8string();

        m_SettingsPath.assign(
            utf8Path.begin(),
            utf8Path.end());

        ImGui::GetIO().IniFilename = m_SettingsPath.c_str();

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

        ImGuiID rightTop = 0;
        ImGuiID rightBottom = 0;

        ImGui::DockBuilderSplitNode(
            right,
            ImGuiDir_Up,
            0.30f,
            &rightTop,
            &rightBottom);

        ImGui::DockBuilderDockWindow("Scene", center);
        ImGui::DockBuilderDockWindow("Hierarchy", leftTop);
        ImGui::DockBuilderDockWindow("Scene File", leftTop);
        ImGui::DockBuilderDockWindow("ForgeSim Debug", leftBottom);
        ImGui::DockBuilderDockWindow("Transform Gizmo", rightTop);
        ImGui::DockBuilderDockWindow("Inspector", rightBottom);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    bool EditorWorkspace::Draw()
    {
        bool resetRequested = false;

        // Append View alongside the existing File menu.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                resetRequested =
                    ImGui::MenuItem("Reset Layout");

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
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