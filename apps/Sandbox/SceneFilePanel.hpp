#pragma once

#include "EditorConsole.hpp"
#include "BundledSamples.hpp"
#include "SamplePreview.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct SceneFilePanelContext
    {
        std::string_view sceneName;
        std::string_view scenePath;
        std::string_view message;

        bool dirty = false;
        bool editingActive = false;
        bool navigationActive = false;
        bool confirmationPending = false;
        std::string_view pendingActionText;
        std::string_view recoveryMessage;
        bool canLocateModel = false;
    };

    enum class SceneFileAction
    {
        None,
        New,
        Open,
        OpenSample,
        BrowseExternalSample,
        OpenExternalSample,
        Save,
        SaveAs,
        SaveAndContinue,
        DiscardAndContinue,
        LocateMissingAssets,
        LocateMissingModel,
        RetryMissingAssets,
        CancelPending
    };

    struct SceneFileRequest
    {
        SceneFileAction action = SceneFileAction::None;
        std::string path;
        bool allowTextureFallbacks = true;
    };

    struct SceneToolbarContext
    {
        bool actionsAllowed = false;
        bool hasScenePath = false;
        bool canUndo = false;
        bool canRedo = false;
        std::string_view sceneName;
        bool dirty = false;
    };

    struct SceneToolbarRequest
    {
        bool undo = false;
        bool redo = false;
    };

    class SceneFilePanel final
    {
    public:
        [[nodiscard]] bool ConsoleWantsKeyboard() const noexcept { return m_Console.WantsKeyboard() || m_SampleBrowserFocused; }

        void RegisterConsoleCommand(std::string name, std::string description, std::string usage,
            std::size_t minimumArguments, std::size_t maximumArguments, ConsoleCommands::Handler handler)
        {
            m_Console.RegisterCommand(std::move(name), std::move(description), std::move(usage),
                minimumArguments, maximumArguments, std::move(handler));
        }

        void DrawMenu(bool actionsAllowed, bool hasScenePath);
        void ResetSampleBrowserLayout();
        void SetExternalSamplePath(const std::filesystem::path& path);
        // Draw before creating the workspace dockspace.
        [[nodiscard]] SceneToolbarRequest DrawToolbar(const SceneToolbarContext& context);

        [[nodiscard]] SceneFileRequest Draw(
            const SceneFilePanelContext& context);

    private:
        void DrawSampleBrowser(bool actionsAllowed, SceneFileRequest& request);

        EditorConsole m_Console;
        SceneFileAction m_MenuAction = SceneFileAction::None;
        std::string m_SampleFilename;
        std::array<SamplePreview, BundledSamples.size()> m_SamplePreviews;
        bool m_SampleBrowserOpen = false;
        bool m_SampleBrowserFocused = false;
        bool m_ResetSampleBrowserLayout = false;
        bool m_AllowSampleTextureFallbacks = true;
        std::array<char, 4096> m_ExternalSamplePath{};
        std::string m_SampleBrowserMessage;
    };
}