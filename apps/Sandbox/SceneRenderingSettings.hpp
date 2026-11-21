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

    enum class SceneToneMapping { AcesApproximation, Standard };

    struct SceneViewportDisplaySettings
    {
        bool showReferenceGrid = true;
        bool showSceneLights = true;
        bool showObjectBounds = false;
        bool showSelectionHighlight = true;
        bool showTransformGizmo = true;

        bool operator==(const SceneViewportDisplaySettings&) const = default;
    };

    struct SceneRenderingSettings
    {
        glm::vec3 backgroundColor{
            0.032f,
            0.036f,
            0.044f
        };

        float exposure = 1.0f;
        SceneToneMapping toneMapping = SceneToneMapping::AcesApproximation;
        float environmentIntensity = 0.2f;

        bool showEnvironmentBackground = false;
        bool flipEnvironmentVertically = false;
        DirectionalShadowSettings directionalShadows{};

        SceneEnvironmentSource environmentSource =
            SceneEnvironmentSource::BundledStudio;
        std::string environmentAsset{};
        SceneViewportDisplaySettings viewport{};
    };
}