#include "SandboxScene.hpp"
#include "SceneDocument.hpp"
#include "SceneCameraValidation.hpp"
#include "SceneDocumentValidation.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace ForgeSim::Sandbox
{
    SandboxScene::SandboxScene(const SceneDocument& document)
    {
        if (const auto error = ValidateSceneDocument(document))
        {
            throw std::invalid_argument(
                "Cannot construct scene at '" +
                error->property + "': " + error->message);
        }

        m_HiddenObjects.insert(document.hiddenObjectIds.begin(), document.hiddenObjectIds.end());
        m_HiddenLights.insert(document.hiddenLightIds.begin(), document.hiddenLightIds.end());
        m_Name = document.name;
        m_SavedCamera = document.camera;
        m_CameraMovementSpeed = document.cameraMovementSpeed;
        m_CameraBookmarks = document.cameraBookmarks;
        m_RenderingSettings = document.rendering;

        m_LastObjectId = 0;
        m_Objects.reserve(document.objects.size());

        for (const SceneObjectData& object : document.objects)
        {
            m_LastObjectId = std::max(m_LastObjectId, object.id);
            m_Objects.push_back(
                SandboxObject{
                    .id = object.id,
                    .transform = object.transform,
                    .name = object.name,
                    .modelPath = object.modelPath,
                    .nodeOverrides = object.nodeOverrides
                });
        }

        m_DirectionalLight = document.hasDirectionalLight ? document.directionalLight :
            DirectionalLight{.id = InvalidSceneLightId, .enabled = false};
        m_PointLights = document.pointLights;
        m_SpotLights = document.spotLights;

        SceneLightId highestLightId = m_DirectionalLight.id;

        for (const PointLight& light : m_PointLights)
        {
            highestLightId = std::max(highestLightId, light.id);
        }

        for (const SpotLight& light : m_SpotLights)
        {
            highestLightId = std::max(highestLightId, light.id);
        }

        m_NextLightId =
            highestLightId ==
            std::numeric_limits<SceneLightId>::max()
            ? InvalidSceneLightId
            : highestLightId + 1;
    }

    SceneDocument SandboxScene::ToDocument() const
    {
        SceneDocument document;
        document.name = m_Name;
        document.hiddenObjectIds.assign(m_HiddenObjects.begin(), m_HiddenObjects.end());
        document.hiddenLightIds.assign(m_HiddenLights.begin(), m_HiddenLights.end());
        // Canonical ordering keeps dirty tracking stable after hide/show cycles.
        std::sort(document.hiddenObjectIds.begin(), document.hiddenObjectIds.end());
        std::sort(document.hiddenLightIds.begin(), document.hiddenLightIds.end());
        document.camera = m_SavedCamera;
        document.cameraMovementSpeed = m_CameraMovementSpeed;
        document.cameraBookmarks = m_CameraBookmarks;
        document.rendering = m_RenderingSettings;

        document.objects.reserve(m_Objects.size());

        for (const SandboxObject& object : m_Objects)
        {
            document.objects.push_back(
                SceneObjectData{
                    .id = object.id,
                    .name = object.name,
                    .transform = object.transform,
                    .modelPath = object.modelPath,
                    .nodeOverrides = object.nodeOverrides
                });
        }

        document.hasDirectionalLight = HasDirectionalLight();
        document.directionalLight = m_DirectionalLight;
        document.pointLights = m_PointLights;
        document.spotLights = m_SpotLights;

        if (const auto error = ValidateSceneDocument(document))
        {
            throw std::invalid_argument(
                "Cannot capture scene at '" +
                error->property + "': " + error->message);
        }

        return document;
    }

    const std::string& SandboxScene::Name() const noexcept
    {
        return m_Name;
    }
}