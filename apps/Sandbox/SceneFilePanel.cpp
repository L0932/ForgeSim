#include "SceneFilePanel.hpp"
#include "BundledSamples.hpp"
#include "EditorMainToolbar.hpp"
#include "ExternalSample.hpp"
#include "SandboxPaths.hpp"

#include <algorithm>

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

            if (ImGui::BeginMenu("Open Sample"))
            {
                for (const auto& sample : BundledSamples)
                {
                    if (ImGui::MenuItem(sample.label))
                    {
                        m_MenuAction = SceneFileAction::OpenSample;
                        m_SampleFilename = sample.filename;
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Browse Samples..."))
                    m_SampleBrowserOpen = true;
                ImGui::EndMenu();
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

    void SceneFilePanel::SetExternalSamplePath(const std::filesystem::path& path)
    {
        const auto bytes = path.generic_u8string();
        if (bytes.size() >= m_ExternalSamplePath.size())
        {
            m_SampleBrowserMessage = "The selected scene path is too long.";
            return;
        }
        m_ExternalSamplePath.fill('\0');
        std::copy(bytes.begin(), bytes.end(), m_ExternalSamplePath.begin());
        m_SampleBrowserMessage.clear();
    }

    void SceneFilePanel::DrawSampleBrowser(bool actionsAllowed, SceneFileRequest& request)
    {
        m_SampleBrowserFocused = false;
        if (!m_SampleBrowserOpen)
            return;

        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowSize(ImVec2(760.0f, 560.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 240.0f), viewport->WorkSize);
        if (ImGui::Begin("Samples", &m_SampleBrowserOpen))
        {
            m_SampleBrowserFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
            ImGui::TextUnformatted("Bundled samples");
            ImGui::Spacing();
            ImGui::BeginDisabled(!actionsAllowed || request.action != SceneFileAction::None);
            const int columns = ImGui::GetContentRegionAvail().x >= 560.0f ? 2 : 1;
            if (ImGui::BeginTable("##SampleCards", columns, ImGuiTableFlags_SizingStretchSame))
            {
                for (std::size_t index = 0; index < BundledSamples.size(); ++index)
                {
                    const auto& sample = BundledSamples[index];
                    auto& preview = m_SamplePreviews[index];
                    preview.Load(BundledAssetDirectory() / "previews" / sample.preview);
                    ImGui::TableNextColumn();
                    ImGui::PushID(sample.filename);
                    if (ImGui::BeginChild("##Card", ImVec2(0.0f, 0.0f),
                        ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
                    {
                        const float width = ImGui::GetContentRegionAvail().x;
                        const ImVec2 imageSize{width, width / preview.aspect};
                        if (preview.texture)
                        {
                            ImGui::Image(ImTextureRef{static_cast<ImTextureID>(preview.texture->Handle())}, imageSize);
                        }
                        else
                        {
                            const auto position = ImGui::GetCursorScreenPos();
                            ImGui::GetWindowDrawList()->AddRectFilled(position,
                                ImVec2(position.x + imageSize.x, position.y + imageSize.y),
                                ImGui::GetColorU32(ImGuiCol_FrameBg));
                            ImGui::GetWindowDrawList()->AddText(
                                ImVec2(position.x + 8.0f, position.y + 8.0f),
                                ImGui::GetColorU32(ImGuiCol_TextDisabled), "Preview unavailable");
                            ImGui::Dummy(imageSize);
                        }
                        ImGui::Spacing();
                        ImGui::TextWrapped("%s", sample.label);
                        ImGui::TextWrapped("%s", sample.description);
                        if (ImGui::Button("Open scene", ImVec2(-1.0f, 0.0f)))
                        {
                            request.action = SceneFileAction::OpenSample;
                            request.path = sample.filename;
                            m_SampleBrowserOpen = false;
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Open a local scene"))
            {
                ImGui::TextWrapped("Choose a ForgeSim scene file and its asset folder.");
                ImGui::TextUnformatted("Scene file (.json)");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputText("##SampleScene", m_ExternalSamplePath.data(), m_ExternalSamplePath.size());
                if (ImGui::Button("Browse scene..."))
                    request.action = SceneFileAction::BrowseExternalSample;
                ImGui::TextUnformatted("Project root");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputText("##SampleRoot", m_ExternalSampleRoot.data(), m_ExternalSampleRoot.size());
                ImGui::TextWrapped("The project root is the folder used to resolve relative asset paths in the scene.");
                ImGui::BeginDisabled(m_ExternalSamplePath.front() == '\0' || m_ExternalSampleRoot.front() == '\0');
                if (ImGui::Button("Open local scene"))
                {
                    try
                    {
                        const auto location = ResolveExternalSample(m_ExternalSamplePath.data(), m_ExternalSampleRoot.data());
                        const auto scene = location.scene.generic_u8string();
                        const auto root = location.root.generic_u8string();
                        request.action = SceneFileAction::OpenExternalSample;
                        request.path.assign(scene.begin(), scene.end());
                        request.projectRoot.assign(root.begin(), root.end());
                        m_SampleBrowserMessage.clear();
                        m_SampleBrowserOpen = false;
                    }
                    catch (const std::exception& error)
                    {
                        m_SampleBrowserMessage = error.what();
                    }
                }
                ImGui::EndDisabled();
                if (!m_SampleBrowserMessage.empty())
                    ImGui::TextWrapped("%s", m_SampleBrowserMessage.c_str());
                ImGui::TextWrapped("Opens a copy. Saving asks for a destination for your scene file.");
            }
            ImGui::EndDisabled();
        }
        ImGui::End();
    }

    SceneToolbarRequest SceneFilePanel::DrawToolbar(const SceneToolbarContext& context)
    {
        const auto result = DrawEditorMainToolbar(context);
        if (result.fileAction != SceneFileAction::None)
            m_MenuAction = result.fileAction;
        return {result.undo, result.redo};
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
            if (menuAction == SceneFileAction::OpenSample)
                request.path = m_SampleFilename;
        }

        m_Console.Draw(context.sceneName, context.scenePath, context.dirty);
        DrawSampleBrowser(actionsAllowed, request);

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