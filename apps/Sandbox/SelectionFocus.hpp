#pragma once

#include "SceneBounds.hpp"
#include "SceneSelection.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <glm/geometric.hpp>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline std::optional<Renderer::AxisAlignedBoundingBox> SelectionFocusBounds(
        std::span<const SandboxObject> objects, const SceneLightingView& lights,
        const SceneSelection& selection, const Renderer::AxisAlignedBoundingBox& fallbackModelBounds)
    {
        if (const auto id = selection.SelectedObjectId())
        {
            for (const auto& object : objects)
                if (object.id == *id)
                    return CalculateSceneBounds(std::span<const SandboxObject>{&object, 1}, fallbackModelBounds);
            return std::nullopt;
        }
        const auto id = selection.SelectedLightId();
        if (!id || *id == InvalidSceneLightId) return std::nullopt;
        const auto world = CalculateSceneBounds(objects, fallbackModelBounds);
        const float sceneRadius = glm::length(world.maximum * 0.5f - world.minimum * 0.5f);
        // Focus a useful neighborhood of the marker rather than a potentially
        // enormous influence volume. Disabled lights can still be inspected.
        const float markerRadius = std::clamp(sceneRadius * 0.05f, 0.01f, 10.0f);
        const auto aroundLight = [&](const glm::vec3& position, float radius)
        {
            return Renderer::AxisAlignedBoundingBox{position - glm::vec3{radius}, position + glm::vec3{radius}};
        };
        if (lights.directionalLight.id == *id)
            return aroundLight(lights.directionalLight.position, markerRadius);
        const auto focusLocalLight = [&](const auto& light)
        {
            const float radius = std::clamp(light.range * 0.1f, markerRadius, markerRadius * 4.0f);
            return aroundLight(light.position, radius);
        };
        for (const auto& light : lights.pointLights)
            if (light.id == *id) return focusLocalLight(light);
        for (const auto& light : lights.spotLights)
            if (light.id == *id) return focusLocalLight(light);
        return std::nullopt;
    }
}
