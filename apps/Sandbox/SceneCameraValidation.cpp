#include "SceneCameraValidation.hpp"

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        [[nodiscard]] bool IsFinite(
            const glm::vec3& value) noexcept
        {
            return
                std::isfinite(value.x) &&
                std::isfinite(value.y) &&
                std::isfinite(value.z);
        }

        [[nodiscard]] bool IsUnitDirection(
            const glm::vec3& value) noexcept
        {
            if (!IsFinite(value))
            {
                return false;
            }

            const glm::dvec3 precise{ value };
            const double lengthSquared =
                glm::dot(precise, precise);

            return std::abs(lengthSquared - 1.0) <= 0.001;
        }
    }

    std::optional<CameraValidationError> ValidateCameraBookmarkName(
        std::string_view name) noexcept
    {
        if (name.empty() || name.size() > 128 ||
            std::all_of(name.begin(), name.end(), [](char c) { return c == ' '; }) ||
            std::any_of(name.begin(), name.end(), [](unsigned char c)
            {
                return c < 32 || c == 127;
            }))
        {
            return CameraValidationError{
                "name", "Bookmark names must contain 1 to 128 UTF-8 bytes, "
                "include a non-space character, and contain no control characters."
            };
        }
        return std::nullopt;
    }

    std::optional<CameraValidationError> ValidateCameraMovementSpeed(float speed) noexcept
    {
        if (!std::isfinite(speed) || speed < 0.0001f || speed > 10000.0f)
            return CameraValidationError{"cameraMovementSpeed",
                "Camera speed must be finite and between 0.0001 and 10000 units per second."};
        return std::nullopt;
    }

    std::optional<CameraValidationError> ValidateSceneCamera(
        const SceneCameraData& camera) noexcept
    {
        if (!IsFinite(camera.position))
        {
            return CameraValidationError{
                "position",
                "Position components must be finite."
            };
        }

        if (!IsUnitDirection(camera.forwardDirection))
        {
            return CameraValidationError{
                "forwardDirection",
                "Forward direction must be finite and unit length."
            };
        }

        if (!IsUnitDirection(camera.upDirection))
        {
            return CameraValidationError{
                "upDirection",
                "Up direction must be finite and unit length."
            };
        }

        const glm::dvec3 right =
            glm::cross(
                glm::dvec3{ camera.forwardDirection },
                glm::dvec3{ camera.upDirection });

        if (glm::dot(right, right) <= 0.000001)
        {
            return CameraValidationError{
                "upDirection",
                "Up direction must not be parallel or nearly "
                "parallel to the forward direction."
            };
        }

        const float fieldOfView =
            camera.verticalFieldOfViewRadians;

        if (!std::isfinite(fieldOfView) ||
            fieldOfView <= 0.0f ||
            fieldOfView >= glm::radians(180.0f))
        {
            return CameraValidationError{
                "verticalFieldOfViewRadians",
                "Vertical field of view must be finite and "
                "strictly between zero and pi radians."
            };
        }

        if (!std::isfinite(camera.nearClippingPlane) ||
            camera.nearClippingPlane <= 0.0f)
        {
            return CameraValidationError{
                "nearClippingPlane",
                "Near clipping distance must be finite and positive."
            };
        }

        if (!std::isfinite(camera.farClippingPlane) ||
            camera.farClippingPlane <= camera.nearClippingPlane)
        {
            return CameraValidationError{
                "farClippingPlane",
                "Far clipping distance must be finite and "
                "greater than the near clipping distance."
            };
        }

        if (camera.referenceViewport &&
            (camera.referenceViewport->width == 0 ||
                camera.referenceViewport->height == 0))
        {
            return CameraValidationError{
                "referenceViewport", "Reference viewport dimensions must be positive."
            };
        }

        // The current renderer represents orientation as target - position.
        // Reject a pose whose unit-length target offset disappears at float precision.
        const glm::vec3 target = camera.position + glm::normalize(camera.forwardDirection);
        const glm::vec3 difference = target - camera.position;
        const float distanceSquared = glm::dot(difference, difference);
        if (!IsFinite(target) || !std::isfinite(distanceSquared) || distanceSquared <= 0.0f)
        {
            return CameraValidationError{
                "position", "Position exceeds the precision supported by camera restoration."
            };
        }

        return std::nullopt;
    }
}