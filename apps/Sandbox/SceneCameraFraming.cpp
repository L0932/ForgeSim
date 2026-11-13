#include "SceneCameraFraming.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    Renderer::PerspectiveCamera CreateSelectionFocusCamera(
        const Renderer::AxisAlignedBoundingBox& selectionBounds,
        const Renderer::AxisAlignedBoundingBox& sceneBounds,
        const Renderer::PerspectiveCamera& currentCamera,
        float aspectRatio)
    {
        if (!std::isfinite(aspectRatio) || aspectRatio <= 0.0f)
            throw std::invalid_argument("Focus requires a positive finite aspect ratio.");
        const auto validate = [](const auto& bounds)
        {
            for (int axis = 0; axis < 3; ++axis)
                if (!std::isfinite(bounds.minimum[axis]) || !std::isfinite(bounds.maximum[axis]) ||
                    bounds.minimum[axis] > bounds.maximum[axis])
                    throw std::invalid_argument("Focus requires finite ordered bounds.");
        };
        validate(selectionBounds);
        validate(sceneBounds);
        const auto cornerOf = [](const auto& bounds, unsigned int corner)
        {
            return glm::vec3{(corner & 1u) ? bounds.maximum.x : bounds.minimum.x,
                (corner & 2u) ? bounds.maximum.y : bounds.minimum.y,
                (corner & 4u) ? bounds.maximum.z : bounds.minimum.z};
        };
        const glm::vec3 center = selectionBounds.minimum * 0.5f + selectionBounds.maximum * 0.5f;
        const auto forward = currentCamera.ForwardDirection();
        const auto right = currentCamera.RightDirection();
        const auto screenUp = glm::normalize(glm::cross(right, forward));
        const float verticalFov = currentCamera.VerticalFieldOfViewRadians();
        const float tangentY = std::tan(verticalFov * 0.5f);
        const float tangentX = tangentY * aspectRatio;
        if (!std::isfinite(tangentY) || tangentY <= 0 || !std::isfinite(tangentX) || tangentX <= 0)
            throw std::invalid_argument("Focus requires a valid field of view.");
        const float radius = glm::length(selectionBounds.maximum * 0.5f - selectionBounds.minimum * 0.5f);
        const float nearPlane = std::min(currentCamera.NearClippingPlane(),
            std::clamp(radius * 0.0001f, 0.0001f, 0.05f));
        float distance = std::max(0.001f, nearPlane * 2.0f);
        // Fit each corner against the actual horizontal/vertical frustum planes.
        // A bounding sphere wastes substantial space for long, flat selections.
        for (unsigned int corner = 0; corner < 8; ++corner)
        {
            const auto offset = cornerOf(selectionBounds, corner) - center;
            const float depth = glm::dot(offset, forward);
            distance = std::max({distance, std::abs(glm::dot(offset, right)) * 1.1f / tangentX - depth,
                std::abs(glm::dot(offset, screenUp)) * 1.1f / tangentY - depth, nearPlane * 2.0f - depth});
        }
        const auto position = center - forward * distance;
        // Any point in the scene remains inside far clipping when rotating in
        // place. Never shrink an existing far plane merely to focus a small light.
        float farPlane = currentCamera.FarClippingPlane();
        for (const auto& bounds : {selectionBounds, sceneBounds})
            for (unsigned int corner = 0; corner < 8; ++corner)
                farPlane = std::max(farPlane, glm::length(cornerOf(bounds, corner) - position) * 1.1f);
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
            !std::isfinite(farPlane))
            throw std::invalid_argument("Focus exceeds the camera coordinate range.");
        return Renderer::PerspectiveCamera{position, center, currentCamera.UpDirection(),
            verticalFov, nearPlane, farPlane};
    }

    SceneCameraFraming CreateEmptySceneCameraFraming(float aspectRatio)
    {
        const Renderer::AxisAlignedBoundingBox workspace{{-1,-1,-1}, {1,1,1}};
        auto framing = CreateSceneCameraFraming(workspace, aspectRatio);
        // Empty scenes have no asset scale. A workspace range must cover the
        // reference grid and allow navigation instead of clipping at ~12 units.
        framing.camera = Renderer::PerspectiveCamera{framing.camera.Position(), glm::vec3{0.0f},
            framing.camera.UpDirection(), framing.camera.VerticalFieldOfViewRadians(), 0.05f, 1000.0f};
        return framing;
    }

    FreeCameraController CreateSceneCameraController(
        const Renderer::AxisAlignedBoundingBox& bounds,
        std::optional<float> savedMovementSpeed)
    {
        auto controller = CreateSceneCameraFraming(bounds, 1.0f).controller;
        if (savedMovementSpeed) controller.SetMovementSpeed(*savedMovementSpeed);
        return controller;
    }

    SceneCameraFraming CreateSceneCameraFraming(
        const Renderer::AxisAlignedBoundingBox& bounds,
        float aspectRatio)
    {
        if (!std::isfinite(aspectRatio) || aspectRatio <= 0.0f)
        {
            throw std::invalid_argument(
                "Camera framing requires a positive finite aspect ratio.");
        }

        for (int axis = 0; axis < 3; ++axis)
        {
            if (!std::isfinite(bounds.minimum[axis]) ||
                !std::isfinite(bounds.maximum[axis]) ||
                bounds.minimum[axis] > bounds.maximum[axis])
            {
                throw std::invalid_argument(
                    "Camera framing requires valid scene bounds.");
            }
        }

        const glm::vec3 center =
            bounds.minimum * 0.5f + bounds.maximum * 0.5f;

        const float radius =
            std::max(
                glm::length(bounds.maximum - bounds.minimum) * 0.5f,
                0.001f);

        constexpr float verticalFieldOfView =
            glm::radians(45.0f);

        // Fit the bounding sphere inside the narrower field of view.
        const float halfAngle =
            std::atan(
                std::tan(verticalFieldOfView * 0.5f) *
                std::min(aspectRatio, 1.0f));

        const float distance =
            radius / std::sin(halfAngle) * 1.25f;

        const float nearPlane =
            std::clamp(radius * 0.0001f, 0.0001f, 0.05f);

        const float farPlane =
            std::max(distance + radius * 4.0f, 10.0f);

        const glm::vec3 position =
            center + glm::vec3{ 0.0f, 0.0f, distance };

        if (!std::isfinite(position.x) ||
            !std::isfinite(position.y) ||
            !std::isfinite(position.z) ||
            !std::isfinite(farPlane))
        {
            throw std::invalid_argument(
                "Scene bounds exceed the camera framing range.");
        }

        return SceneCameraFraming{
            .camera = Renderer::PerspectiveCamera{
                position,
                center,
                glm::vec3{ 0.0f, 1.0f, 0.0f },
                verticalFieldOfView,
                nearPlane,
                farPlane
            },
            .controller = FreeCameraController{
                FreeCameraControllerSpecification{
                    .movementSpeed = 10.0f,
                    .mouseSensitivity = 0.002f,
                    .maximumDeltaSeconds = 0.1f
                }
            }
        };
    }
}