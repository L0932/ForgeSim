#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/vec3.hpp>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
    struct LightDebugLayout
    {
        glm::vec3 directionalAnchor;
        float characteristicSize;
        float directionalLength;
        float markerSize;
        float pickRadius;
    };

    [[nodiscard]] inline LightDebugLayout CalculateLightDebugLayout(
        const Renderer::AxisAlignedBoundingBox& modelBounds)
    {
        const glm::vec3 extent =
            modelBounds.maximum - modelBounds.minimum;

        const glm::vec3 center =
            modelBounds.minimum * 0.5f +
            modelBounds.maximum * 0.5f;

        const auto isFinite =
            [](const glm::vec3& value)
            {
                return
                    std::isfinite(value.x) &&
                    std::isfinite(value.y) &&
                    std::isfinite(value.z);
            };

        if (!isFinite(extent) ||
            !isFinite(center) ||
            extent.x < 0.0f ||
            extent.y < 0.0f ||
            extent.z < 0.0f)
        {
            throw std::invalid_argument(
                "Light debug layout requires finite, ordered bounds.");
        }

        const float characteristicSize =
            std::max({
                extent.x,
                extent.y,
                extent.z,
                0.001f
                });

        const glm::vec3 directionalAnchor =
            center +
            glm::vec3{
                0.0f,
                extent.y * 0.5f + characteristicSize * 0.35f,
                0.0f
        };

        if (!isFinite(directionalAnchor))
        {
            throw std::invalid_argument(
                "Light debug anchor exceeds the supported numeric range.");
        }

        return LightDebugLayout{
            .directionalAnchor = directionalAnchor,
            .characteristicSize = characteristicSize,
            .directionalLength = characteristicSize * 0.20f,
            .markerSize = characteristicSize * 0.015f,
            .pickRadius = characteristicSize * 0.025f
        };
    }
}