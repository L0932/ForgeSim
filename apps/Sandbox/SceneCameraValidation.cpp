#include "SceneCameraValidation.hpp"

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

        return std::nullopt;
    }
}