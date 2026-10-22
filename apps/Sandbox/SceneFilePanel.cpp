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

        const bool savingAllowed =
            !context.editingActive &&
            !context.navigationActive;

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

        const bool actionsAllowed =
            savingAllowed && !context.confirmationPending;

        const SceneFileAction menuAction =
            std::exchange(m_MenuAction, SceneFileAction::None);

        bool openRequested = false;
        bool saveAsRequested = false;

        if (actionsAllowed)
        {
            switch (menuAction)
            {
            case SceneFileAction::New:
            case SceneFileAction::Save:
                request.action = menuAction;
                break;

            case SceneFileAction::Open:
                openRequested = true;
                break;

            case SceneFileAction::SaveAs:
                saveAsRequested = true;
                break;

            default:
                break;
            }
        }

        const ImGuiIO& io = ImGui::GetIO();

        if (request.action == SceneFileAction::None &&
            !openRequested &&
            !saveAsRequested &&
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
            if (context.scenePath.empty())
            {
                saveAsRequested = true;
            }
            else
            {
                request.action = SceneFileAction::Save;
            }
        }

        constexpr const char* openPopup =
            "Open Scene###SceneOpenDialog";

        constexpr const char* savePopup =
            "Save Scene As###SceneSaveAsDialog";

        if (openRequested)
        {
            ImGui::OpenPopup(openPopup);
        }

        if (saveAsRequested)
        {
            ImGui::OpenPopup(savePopup);
        }

        if (ImGui::BeginPopupModal(
            openPopup,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText(
                "Scene path",
                m_OpenPath,
                sizeof(m_OpenPath));

            ImGui::TextDisabled(
                "Relative paths use the project's scenes folder.");

            ImGui::BeginDisabled(
                !actionsAllowed || m_OpenPath[0] == '\0');

            if (ImGui::Button("Open"))
            {
                request.action = SceneFileAction::Open;
                request.path = m_OpenPath;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndDisabled();
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal(
            savePopup,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText(
                "Save path",
                m_SaveAsPath,
                sizeof(m_SaveAsPath));

            ImGui::TextDisabled(
                "Relative paths use the project's scenes folder.");

            ImGui::BeginDisabled(
                !actionsAllowed || m_SaveAsPath[0] == '\0');

            if (ImGui::Button("Save"))
            {
                request.action = SceneFileAction::SaveAs;
                request.path = m_SaveAsPath;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndDisabled();
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
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

            const bool needsPath = context.scenePath.empty();

            if (needsPath)
            {
                ImGui::InputText(
                    "Save path",
                    m_SaveAsPath,
                    sizeof(m_SaveAsPath));

                ImGui::TextDisabled(
                    "Relative paths use the project's scenes folder.");
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
                !savingAllowed ||
                (needsPath && m_SaveAsPath[0] == '\0'));

            if (ImGui::Button("Save and Continue"))
            {
                request.action = SceneFileAction::SaveAndContinue;

                if (needsPath)
                {
                    request.path = m_SaveAsPath;
                }

                // Keep the popup open until saving succeeds.
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