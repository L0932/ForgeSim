#pragma once

#include <ForgeSim/Renderer/Bounds.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Renderer
{
    // Fits finite, ordered world-space bounds inside a padded sphere.
    // The sphere has a minimum radius of 0.5 world units so point-sized
    // geometry still produces a usable projection.
    //
    // directionToLight points from a surface toward the light. It must be
    // finite and nonzero; its magnitude does not affect the projection.
    // The returned matrix uses right-handed OpenGL [-1, 1] clip depth.
    // No graphics context is required. Invalid inputs or an unrepresentable
    // result throw std::invalid_argument.

    // The sphere has a minimum radius of 1e-6 world units to avoid
// a zero-sized projection for degenerate bounds.
    [[nodiscard]] glm::mat4 CreateDirectionalShadowMatrix(
        const AxisAlignedBoundingBox& worldBounds,
        const glm::vec3& directionToLight);
}
