#include "SceneFilePanel.hpp"

#include <string>
#include <utility>

#include <imgui.h>

namespace ForgeSim::Sandbox
{
    void SceneFilePanel::DrawMenu(
        bool actionsAllowed,
        bool hasScenePath)
    {
        if (!ImGui::BeginMainMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu("File"))
        {
            ImGui::BeginDisabled(!actionsAllowed);

            if (ImGui::MenuItem("New Scene"))
            {
                m_MenuAction = SceneFileAction::New;
            }

            if (ImGui::MenuItem("Open Scene..."))
            {
                m_MenuAction = SceneFileAction::Open;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                m_MenuAction =
                    hasScenePath
                    ? SceneFileAction::Save
                    : SceneFileAction::SaveAs;
            }

            if (ImGui::MenuItem("Save As..."))
            {
                m_MenuAction = SceneFileAction::SaveAs;
            }

            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    SceneFileRequest SceneFilePanel::Draw(
        const SceneFilePanelContext& context)
    {
        SceneFileRequest request;

        const bool actionsAllowed =
            !context.editingActive &&
            !context.navigationActive &&
            !context.confirmationPending;

        const SceneFileAction menuAction =
            std::exchange(m_MenuAction, SceneFileAction::None);

        if (actionsAllowed)
        {
            request.action = menuAction;
        }

        if (ImGui::Begin("Scene File"))
        {
            const std::string title =
                std::string{ context.sceneName } +
                (context.dirty ? " *" : "");

            ImGui::TextUnformatted(title.c_str());

            if (context.scenePath.empty())
            {
                ImGui::TextDisabled("Not saved to a file.");
            }
            else
            {
                const std::string path{ context.scenePath };
                ImGui::TextWrapped("%s", path.c_str());
            }

            if (!context.message.empty())
            {
                const std::string message{ context.message };
                ImGui::TextWrapped("%s", message.c_str());
            }
        }

        ImGui::End();

        const ImGuiIO& io = ImGui::GetIO();

        if (request.action == SceneFileAction::None &&
            actionsAllowed &&
            !ImGui::IsPopupOpen(
                nullptr,
                ImGuiPopupFlags_AnyPopupId |
                ImGuiPopupFlags_AnyPopupLevel) &&
            !io.WantTextInput &&
            !ImGui::IsAnyItemActive() &&
            io.KeyCtrl &&
            !io.KeyAlt &&
            !io.KeyShift &&
            ImGui::IsKeyPressed(ImGuiKey_S, false))
        {
            request.action = SceneFileAction::Save;
        }

        constexpr const char* popupName =
            "Unsaved changes###SceneCloseConfirmation";

        if (context.confirmationPending &&
            !ImGui::IsPopupOpen(popupName))
        {
            ImGui::OpenPopup(popupName);
        }

        if (ImGui::BeginPopupModal(
            popupName,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (!context.confirmationPending)
            {
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return request;
            }

            const std::string prompt =
                "Save your changes before " +
                std::string{ context.pendingActionText } + "?";

            ImGui::TextUnformatted(prompt.c_str());

            if (context.scenePath.empty())
            {
                ImGui::TextDisabled(
                    "Save and Continue will open a Save dialog.");
            }
            else
            {
                const std::string path{ context.scenePath };
                ImGui::TextWrapped("%s", path.c_str());
            }

            if (!context.message.empty())
            {
                const std::string message{ context.message };
                ImGui::TextWrapped("%s", message.c_str());
            }

            ImGui::Separator();

            ImGui::BeginDisabled(
                context.editingActive || context.navigationActive);

            if (ImGui::Button("Save and Continue"))
            {
                request.action = SceneFileAction::SaveAndContinue;
            }

            ImGui::EndDisabled();
            ImGui::SameLine();

            if (ImGui::Button("Discard"))
            {
                request.action = SceneFileAction::DiscardAndContinue;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                request.action = SceneFileAction::CancelPending;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return request;
    }
}