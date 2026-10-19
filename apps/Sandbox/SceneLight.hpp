#pragma once

#include <cstdint>
#include <span>

#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    using SceneLightId = std::uint64_t; 

    // Zero represents an unassigned light ID
    inline constexpr SceneLightId InvalidSceneLightId = 0;

    struct DirectionalLight
    {
        SceneLightId id = InvalidSceneLightId;
        bool enabled = true;

        // World-space editor position; does not affect illumination.
        glm::vec3 position{ 0.0f };

        // World-space direction from the surface toward the light.
        glm::vec3 directionToLight{
            -0.5f,
            1.0f,
            0.3f
        };

        glm::vec3 color{ 1.0f };
        float intensity = 3.0f;
    };

    struct PointLight
    {
        SceneLightId id = InvalidSceneLightId;
        bool enabled = true;

        // World-space position
        glm::vec3 position{ 0.0f };

        glm::vec3 color{ 1.0f };
        float intensity = 1.0f;

        // Positive influence distance in world units.
        float range = 10.0f;
    };

    struct SpotLight
    {
        SceneLightId id = InvalidSceneLightId;
        bool enabled = true;

        // World-space position.
        glm::vec3 position{ 0.0f };

        // World-space direction in which the light emits.
        // Must be finite and nonzero; consumers normalize as needed.
        glm::vec3 emissionDirection{
            0.0f,
           -1.0f,
            0.0f
        };

        glm::vec3 color{ 1.0f };
        float intensity = 1.0f;

        // Positive influence distance in world units.
        float range = 10.0f;

        // Cone half-angles measured from emissionDirection.
        // Require:
        // 0 <= innerConeAngleRadians < outerConeAngleRadians < pi / 2.
        float innerConeAngleRadians = 0.2617994f; // 15 degrees
        float outerConeAngleRadians = 0.5235988f; // 30 degrees
    };

    // Borrows scene light data for the duration of rendering.
    struct SceneLightingView
    {
        const DirectionalLight& directionalLight;
        std::span<const PointLight> pointLights;
        std::span<const SpotLight> spotLights;
    };
}