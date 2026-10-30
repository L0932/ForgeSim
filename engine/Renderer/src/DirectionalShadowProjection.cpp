#include <ForgeSim/Renderer/DirectionalShadowProjection.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

namespace ForgeSim::Renderer
{
    glm::mat4 CreateDirectionalShadowMatrix(
        const AxisAlignedBoundingBox& worldBounds,
        const glm::vec3& directionToLight)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            if (!std::isfinite(worldBounds.minimum[axis]) ||
                !std::isfinite(worldBounds.maximum[axis]) ||
                worldBounds.minimum[axis] > worldBounds.maximum[axis])
            {
                throw std::invalid_argument(
                    "Directional shadow bounds must be finite and ordered.");
            }

            if (!std::isfinite(directionToLight[axis]))
            {
                throw std::invalid_argument(
                    "Directional shadow direction must be finite and nonzero.");
            }
        }

        const glm::dvec3 inputDirection{ directionToLight };
        const double largestComponent = std::max({
            std::abs(inputDirection.x),
            std::abs(inputDirection.y),
            std::abs(inputDirection.z)
        });

        if (largestComponent == 0.0)
        {
            throw std::invalid_argument(
                "Directional shadow direction must be finite and nonzero.");
        }

        // Scaling before normalization avoids overflow/underflow when a
        // valid direction has an unusually large or small magnitude.
        const glm::dvec3 direction = glm::normalize(
            inputDirection / largestComponent);

        // Double intermediates avoid overflowing float bounds arithmetic.
        const glm::dvec3 minimum{ worldBounds.minimum };
        const glm::dvec3 maximum{ worldBounds.maximum };
        const glm::dvec3 center = (minimum + maximum) * 0.5;
        const double radius = std::max(
            glm::length(maximum - minimum) * 0.55,
            1.0e-6);

        // A camera cannot look along its own up vector. Choose another
        // reference axis for lights pointing nearly straight up or down.
        const glm::dvec3 up = std::abs(direction.y) > 0.99
            ? glm::dvec3{ 0.0, 0.0, 1.0 }
            : glm::dvec3{ 0.0, 1.0, 0.0 };

        const glm::dvec3 eye = center + direction * (2.0 * radius);
        const glm::dmat4 view = glm::lookAtRH(eye, center, up);
        const glm::dmat4 projection = glm::orthoRH_NO(
            -radius, radius, -radius, radius,
            radius * 0.01, radius * 4.0);
        const glm::dmat4 preciseMatrix = projection * view;

        // Validate before narrowing to the float matrix consumed by GLSL.
        glm::mat4 result{ 1.0f };
        for (int column = 0; column < 4; ++column)
        {
            for (int row = 0; row < 4; ++row)
            {
                const double value = preciseMatrix[column][row];
                constexpr double maximumFloat =
                    static_cast<double>(std::numeric_limits<float>::max());

                if (!std::isfinite(value) || std::abs(value) > maximumFloat)
                {
                    throw std::invalid_argument(
                        "Directional shadow projection exceeds the supported coordinate range.");
                }

                result[column][row] = static_cast<float>(value);
            }
        }

        return result;
    }
}
