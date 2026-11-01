#pragma once

#include "DirectionalShadowSettings.hpp"

#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    struct SceneRenderingSettings
    {
        glm::vec3 backgroundColor{
            0.032f,
            0.036f,
            0.044f
        };

        float exposure = 1.0f;
        float environmentIntensity = 0.2f;

        bool showEnvironmentBackground = false;
        DirectionalShadowSettings directionalShadows;
    };
}