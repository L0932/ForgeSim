#pragma once

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
        void DrawMenu(bool actionsAllowed, bool hasScenePath);
        // Draw before creating the workspace dockspace.
        [[nodiscard]] SceneToolbarRequest DrawToolbar(const SceneToolbarContext& context);

        [[nodiscard]] SceneFileRequest Draw(
            const SceneFilePanelContext& context);

    private:
        SceneFileAction m_MenuAction = SceneFileAction::None;
    };
}