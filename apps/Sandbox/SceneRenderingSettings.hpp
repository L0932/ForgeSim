#pragma once

#include "DirectionalShadowSettings.hpp"

#include <string>

#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
    enum class SceneEnvironmentSource
    {
        BundledStudio,
        None,
        ProjectAsset
    };

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
        DirectionalShadowSettings directionalShadows{};

        SceneEnvironmentSource environmentSource =
            SceneEnvironmentSource::BundledStudio;
        std::string environmentAsset{};
    };
}