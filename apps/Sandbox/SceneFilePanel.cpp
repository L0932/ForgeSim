#include "SceneFilePanel.hpp"

#include <string>

#include <imgui.h>

namespace ForgeSim::Sandbox
{
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

            ImGui::BeginDisabled(
                !savingAllowed || context.confirmationPending);

            if (ImGui::Button("New Scene"))
            {
                request.action = SceneFileAction::New;
            }

            ImGui::SameLine();

            if (ImGui::Button("Save (Ctrl+S)"))
            {
                request.action = SceneFileAction::Save;
            }

            ImGui::Separator();

            ImGui::InputText(
                "Open path",
                m_OpenPath,
                sizeof(m_OpenPath));

            ImGui::TextDisabled(
                "Relative paths open from the project's scenes folder.");

            ImGui::BeginDisabled(m_OpenPath[0] == '\0');

            if (ImGui::Button("Open Scene"))
            {
                request.action = SceneFileAction::Open;
                request.path = m_OpenPath;
            }

            ImGui::EndDisabled();

            ImGui::Separator();

            ImGui::InputText(
                "Save As path",
                m_SaveAsPath,
                sizeof(m_SaveAsPath));

            ImGui::TextDisabled(
                "Relative paths save inside the project's scenes folder.");

            ImGui::BeginDisabled(m_SaveAsPath[0] == '\0');

            if (ImGui::Button("Save As"))
            {
                request.action = SceneFileAction::SaveAs;
                request.path = m_SaveAsPath;
            }

            ImGui::EndDisabled();
            ImGui::EndDisabled();

            if (!context.message.empty())
            {
                const std::string message{ context.message };
                ImGui::TextWrapped("%s", message.c_str());
            }
        }

        ImGui::End();

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

        const ImGuiIO& io = ImGui::GetIO();

        if (request.action == SceneFileAction::None &&
            !context.confirmationPending &&
            savingAllowed &&
            !io.WantTextInput &&
            !ImGui::IsAnyItemActive() &&
            io.KeyCtrl &&
            !io.KeyAlt &&
            !io.KeyShift &&
            ImGui::IsKeyPressed(ImGuiKey_S, false))
        {
            request.action = SceneFileAction::Save;
        }

        return request;
    }
}