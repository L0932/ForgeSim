#pragma once

#include "FreeCameraController.hpp"
#include <optional>

#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneCameraFraming
    {
        Renderer::PerspectiveCamera camera;
        FreeCameraController controller;
    };

    // Restore navigation independently of the saved camera pose.
    [[nodiscard]] FreeCameraController CreateSceneCameraController(
        const Renderer::AxisAlignedBoundingBox& bounds,
        std::optional<float> savedMovementSpeed = std::nullopt);

    [[nodiscard]] SceneCameraFraming CreateSceneCameraFraming(
        const Renderer::AxisAlignedBoundingBox& bounds,
        float aspectRatio);
}