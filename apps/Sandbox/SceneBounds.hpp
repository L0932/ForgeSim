#pragma once

#include "SandboxObject.hpp"

#include <span>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
    // All objects currently share the same model-local bounds.
    // Empty scenes return the supplied model bounds.
    [[nodiscard]] Renderer::AxisAlignedBoundingBox
        CalculateSceneBounds(
            std::span<const SandboxObject> objects,
            const Renderer::AxisAlignedBoundingBox& modelBounds);
}