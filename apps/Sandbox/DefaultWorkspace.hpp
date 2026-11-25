#pragma once

#include "SceneDocument.hpp"

namespace ForgeSim::Sandbox
{
    // Startup and New Scene share an asset-free, unsaved authoring workspace.
    // Camera framing is provided by CreateEmptySceneCameraFraming, so this does
    // not create an authored camera or inherit cameras from the previous scene.
    [[nodiscard]] inline SceneDocument CreateDefaultWorkspace()
    {
        SceneDocument document;
        document.rendering.environmentSource =
            SceneEnvironmentSource::BundledSky;
        document.rendering.showEnvironmentBackground = true;
        document.rendering.flipEnvironmentVertically = true;
        return document;
    }
}
