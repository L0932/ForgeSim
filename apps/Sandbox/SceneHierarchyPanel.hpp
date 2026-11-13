#pragma once

#include "SandboxScene.hpp"
#include "SceneSelection.hpp"

#include <imgui.h>
#include <functional>
#include <ForgeSim/Assets/SceneAsset.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneHierarchyPanelContext
    {
        SandboxScene& scene;
        SceneSelection& selection;

        bool interactionDisabled = false;
        // Borrow only during Draw; scene replacements invalidate asset pointers.
        std::function<const Assets::SceneAsset*(const std::string&)> inspectModel;
    };

    class SceneHierarchyPanel final
    {
    public:
        void Draw(const SceneHierarchyPanelContext& context);
        [[nodiscard]] std::optional<bool> SelectionVisibilityRequested() const noexcept { return m_SelectionVisibilityRequested; }
        [[nodiscard]] bool DuplicateSelectionRequested() const noexcept { return m_DuplicateSelectionRequested; }
        [[nodiscard]] bool DeleteSelectionRequested() const noexcept { return m_DeleteSelectionRequested; }
        [[nodiscard]] bool FocusSelectionRequested() const noexcept { return m_FocusSelectionRequested; }


    private:
        void DrawLightEntry(
            const char* typeName,
            SceneLightId id,
            bool enabled,
            SceneSelection& selection, SandboxScene& scene);

        std::optional<bool> m_SelectionVisibilityRequested;
        bool m_DeleteSelectionRequested = false;
        bool m_DuplicateSelectionRequested = false;
        bool m_FocusSelectionRequested = false;
        ImGuiTextFilter m_Filter;
    };
}