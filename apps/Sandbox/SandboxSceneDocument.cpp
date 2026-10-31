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

        m_Name = document.name;
        m_SavedCamera = document.camera;

        m_Objects.reserve(document.objects.size());

        for (const SceneObjectData& object : document.objects)
        {
            m_Objects.push_back(
                SandboxObject{
                    .id = object.id,
                    .transform = object.transform,
                    .name = object.name,
                    .modelPath = object.modelPath
                });
        }

        m_DirectionalLight = document.directionalLight;
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
        document.camera = m_SavedCamera;

        document.objects.reserve(m_Objects.size());

        for (const SandboxObject& object : m_Objects)
        {
            document.objects.push_back(
                SceneObjectData{
                    .id = object.id,
                    .name = object.name,
                    .transform = object.transform,
                    .modelPath = object.modelPath
                });
        }

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