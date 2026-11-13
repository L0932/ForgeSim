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

    // Fit the selected bounds in the current view; clipping still covers the scene.
    [[nodiscard]] Renderer::PerspectiveCamera CreateSelectionFocusCamera(
        const Renderer::AxisAlignedBoundingBox& selectionBounds,
        const Renderer::AxisAlignedBoundingBox& sceneBounds,
        const Renderer::PerspectiveCamera& currentCamera,
        float aspectRatio);

    [[nodiscard]] SceneCameraFraming CreateSceneCameraFraming(
        const Renderer::AxisAlignedBoundingBox& bounds,
        float aspectRatio);
}