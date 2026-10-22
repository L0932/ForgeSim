#pragma once

#include "FreeCameraController.hpp"

#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneCameraFraming
    {
        Renderer::PerspectiveCamera camera;
        FreeCameraController controller;
    };

    [[nodiscard]] SceneCameraFraming CreateSceneCameraFraming(
        const Renderer::AxisAlignedBoundingBox& bounds,
        float aspectRatio);
}