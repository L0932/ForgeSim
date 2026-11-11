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


    private:
        void DrawLightEntry(
            const char* typeName,
            SceneLightId id,
            bool enabled,
            SceneSelection& selection, SandboxScene& scene);

        ImGuiTextFilter m_Filter;
    };
}