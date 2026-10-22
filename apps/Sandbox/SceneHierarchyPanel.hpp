#pragma once

#include "SandboxScene.hpp"
#include "SceneSelection.hpp"

namespace ForgeSim::Sandbox
{
    struct SceneHierarchyPanelContext
    {
        const SandboxScene& scene;
        SceneSelection& selection;

        bool interactionDisabled = false;
    };

    class SceneHierarchyPanel final
    {
    public:
        void Draw(const SceneHierarchyPanelContext& context) const;

    private:
        static void DrawLightEntry(
            const char* typeName,
            SceneLightId id,
            bool enabled,
            SceneSelection& selection);
    };
}