#pragma once

#include "SandboxScene.hpp"
#include "SceneSelection.hpp"

#include <imgui.h>

namespace ForgeSim::Sandbox
{
    struct SceneHierarchyPanelContext
    {
        SandboxScene& scene;
        SceneSelection& selection;

        bool interactionDisabled = false;
    };

    class SceneHierarchyPanel final
    {
    public:
        void Draw(const SceneHierarchyPanelContext& context);
        [[nodiscard]] bool DuplicateSelectionRequested() const noexcept { return m_DuplicateSelectionRequested; }
        [[nodiscard]] bool DeleteSelectionRequested() const noexcept { return m_DeleteSelectionRequested; }
        [[nodiscard]] bool FocusSelectionRequested() const noexcept { return m_FocusSelectionRequested; }


    private:
        void DrawLightEntry(
            const char* typeName,
            SceneLightId id,
            bool enabled,
            SceneSelection& selection, SandboxScene& scene);

        bool m_DeleteSelectionRequested = false;
        bool m_DuplicateSelectionRequested = false;
        bool m_FocusSelectionRequested = false;
        ImGuiTextFilter m_Filter;
    };
}