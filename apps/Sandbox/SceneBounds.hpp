#pragma once

#include "SandboxObject.hpp"

#include <span>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
    // Each object uses its loaded model bounds, with a fallback for unloaded objects.
    // Empty scenes return the supplied model bounds.
    [[nodiscard]] Renderer::AxisAlignedBoundingBox
        CalculateSceneBounds(
            std::span<const SandboxObject> objects,
            const Renderer::AxisAlignedBoundingBox& modelBounds);
}