#pragma once

#include "SceneCameraData.hpp"
#include "SceneLight.hpp"
#include "SceneRenderingSettings.hpp"
#include "Transform.hpp"

#include <optional>
#include <cstdint>
#include <string>
#include <vector>

namespace ForgeSim::Sandbox
{
    inline constexpr std::uint32_t CurrentSceneDocumentVersion = 4;

    struct SceneObjectData
    {
        // Zero represents an unassigned object ID.
        std::uint64_t id = 0;

        std::string name;
        Transform transform;

        // Project-relative path using forward slashes.
        // Example: assets/models/BoxTextured.glb
        std::string modelPath;
    };

    struct SceneDocument
    {
        std::uint32_t version = CurrentSceneDocumentVersion;

        std::string name = "Untitled";

        // No saved camera means the application should frame the scene.
        std::optional<SceneCameraData> camera;
        std::vector<SceneCameraBookmark> cameraBookmarks;
        // Missing or null uses the default fly speed of 10 units/s.
        std::optional<float> cameraMovementSpeed;
        SceneRenderingSettings rendering;

        std::vector<SceneObjectData> objects;

        // Editor visibility is separate from authored light enabled settings.
        std::vector<std::uint64_t> hiddenObjectIds;
        std::vector<SceneLightId> hiddenLightIds;

        // Zero or one directional light; absence is serialized as JSON null.
        bool hasDirectionalLight = true;
        DirectionalLight directionalLight{
            .id = 1
        };

        std::vector<PointLight> pointLights;
        std::vector<SpotLight> spotLights;
    };
}