#include "SandboxScene.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <unordered_set>

int main()
{
    using namespace ForgeSim::Sandbox;

    try
    {
        SandboxScene scene;

        if (scene.MainDirectionalLight().id ==
            InvalidSceneLightId)
        {
            std::cerr << "Directional light has an invalid ID\n";
            return EXIT_FAILURE;
        }

        if (!scene.PointLights().empty() ||
            !scene.SpotLights().empty())
        {
            std::cerr << "Expected initially empty light collections\n";
            return EXIT_FAILURE;
        }

        std::unordered_set<SceneLightId> assignedIds{
            scene.MainDirectionalLight().id
        };

        const SceneLightId pointId = scene.AddPointLight();
        const SceneLightId spotId = scene.AddSpotLight();

        if (pointId == InvalidSceneLightId ||
            spotId == InvalidSceneLightId ||
            !assignedIds.insert(pointId).second ||
            !assignedIds.insert(spotId).second)
        {
            std::cerr << "Light IDs must be valid and unique across types\n";
            return EXIT_FAILURE;
        }

        if (scene.PointLights().size() != 1 ||
            scene.SpotLights().size() != 1 ||
            scene.PointLights()[0].id != pointId ||
            scene.SpotLights()[0].id != spotId)
        {
            std::cerr << "Scene did not retain the added lights\n";
            return EXIT_FAILURE;
        }

        scene.MainDirectionalLight().enabled = false;
        scene.MainDirectionalLight().intensity = 4.0f;

        scene.PointLights()[0].position = { 1.0f, 2.0f, 3.0f };
        scene.PointLights()[0].intensity = 7.0f;
        scene.PointLights()[0].range = 12.0f;

        scene.SpotLights()[0].enabled = false;
        scene.SpotLights()[0].intensity = 9.0f;
        scene.SpotLights()[0].innerConeAngleRadians = 0.2f;
        scene.SpotLights()[0].outerConeAngleRadians = 0.5f;

        // Grow both collections. Reacquire views afterward rather
        // than retaining references that insertion could invalidate.
        for (int index = 0; index < 64; ++index)
        {
            const SceneLightId nextPointId = scene.AddPointLight();
            const SceneLightId nextSpotId = scene.AddSpotLight();

            if (nextPointId == InvalidSceneLightId ||
                nextSpotId == InvalidSceneLightId ||
                !assignedIds.insert(nextPointId).second ||
                !assignedIds.insert(nextSpotId).second)
            {
                std::cerr << "Collection growth produced duplicate IDs\n";
                return EXIT_FAILURE;
            }
        }

        const SandboxScene& readOnlyScene = scene;

        if (readOnlyScene.PointLights().size() != 65 ||
            readOnlyScene.SpotLights().size() != 65)
        {
            std::cerr << "Unexpected light collection sizes\n";
            return EXIT_FAILURE;
        }

        const DirectionalLight& directional =
            readOnlyScene.MainDirectionalLight();

        const PointLight& point =
            readOnlyScene.PointLights()[0];

        const SpotLight& spot =
            readOnlyScene.SpotLights()[0];

        if (directional.enabled ||
            directional.intensity != 4.0f ||
            point.id != pointId ||
            point.position.x != 1.0f ||
            point.position.y != 2.0f ||
            point.position.z != 3.0f ||
            point.intensity != 7.0f ||
            point.range != 12.0f ||
            spot.id != spotId ||
            spot.enabled ||
            spot.intensity != 9.0f ||
            spot.innerConeAngleRadians != 0.2f ||
            spot.outerConeAngleRadians != 0.5f)
        {
            std::cerr << "Scene light edits or IDs were not preserved\n";
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Unexpected scene-light exception: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }
}