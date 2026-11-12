#pragma once

#include "EditorConsole.hpp"

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
    };

    enum class SceneFileAction
    {
        None,
        New,
        Open,
        OpenSample,
        Save,
        SaveAs,
        SaveAndContinue,
        DiscardAndContinue,
        CancelPending
    };

    struct SceneFileRequest
    {
        SceneFileAction action = SceneFileAction::None;
        std::string path;
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
        [[nodiscard]] bool ConsoleWantsKeyboard() const noexcept { return m_Console.WantsKeyboard(); }

        void RegisterConsoleCommand(std::string name, std::string description, std::string usage,
            std::size_t minimumArguments, std::size_t maximumArguments, ConsoleCommands::Handler handler)
        {
            m_Console.RegisterCommand(std::move(name), std::move(description), std::move(usage),
                minimumArguments, maximumArguments, std::move(handler));
        }

        void DrawMenu(bool actionsAllowed, bool hasScenePath);
        // Draw before creating the workspace dockspace.
        [[nodiscard]] SceneToolbarRequest DrawToolbar(const SceneToolbarContext& context);

        [[nodiscard]] SceneFileRequest Draw(
            const SceneFilePanelContext& context);

    private:
        EditorConsole m_Console;
        SceneFileAction m_MenuAction = SceneFileAction::None;
        std::string m_SampleFilename;
    };
}