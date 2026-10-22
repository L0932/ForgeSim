#include "SceneCameraFraming.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
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
            std::max(radius * 0.01f, 0.001f);

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
                    .movementSpeed =
                        std::max(distance * 0.5f, 0.01f),
                    .mouseSensitivity = 0.002f,
                    .maximumDeltaSeconds = 0.1f
                }
            }
        };
    }
}