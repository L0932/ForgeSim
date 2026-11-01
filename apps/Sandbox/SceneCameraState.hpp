#pragma once

#include "SceneCameraData.hpp"

#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] SceneCameraData CaptureSceneCamera(
        const Renderer::PerspectiveCamera& camera,
        std::optional<SceneCaptureExtent> referenceViewport = std::nullopt);

    [[nodiscard]] Renderer::PerspectiveCamera CreateSceneCamera(
        const SceneCameraData& camera);
}
