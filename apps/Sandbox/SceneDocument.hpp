#pragma once

#include "SceneLight.hpp"
#include "Transform.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ForgeSim::Sandbox
{
    inline constexpr std::uint32_t CurrentSceneDocumentVersion = 1;

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

        std::vector<SceneObjectData> objects;

        // Match the current runtime's single directional light.
        DirectionalLight directionalLight{
            .id = 1
        };

        std::vector<PointLight> pointLights;
        std::vector<SpotLight> spotLights;
    };
}