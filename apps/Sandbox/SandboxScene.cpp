#include "SandboxScene.hpp"

#include <stdexcept>
#include <utility>
#include <algorithm>
#include <limits>

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Sandbox
{
	SandboxScene::SandboxScene()
		: m_Objects{
			SandboxObject{
				.id = 1,
				.transform = Transform{
					.position = glm::vec3{ 0.0f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{ 0.0f },
					.scale = glm::vec3{ 1.0f }
				}
			}
		}
	{}

    void SandboxScene::SetObjectVisible(SandboxObjectId id, bool visible)
    {
        if (visible) m_HiddenObjects.erase(id);
        else if (FindObject(id)) m_HiddenObjects.insert(id);
    }

    void SandboxScene::SetLightVisible(SceneLightId id, bool visible)
    {
        if (visible) m_HiddenLights.erase(id);
        else if (m_DirectionalLight.id == id ||
            std::any_of(m_PointLights.begin(), m_PointLights.end(),
                [id](const auto& light) { return light.id == id; }) ||
            std::any_of(m_SpotLights.begin(), m_SpotLights.end(),
                [id](const auto& light) { return light.id == id; }))
            m_HiddenLights.insert(id);
    }

	DirectionalLight&
		SandboxScene::MainDirectionalLight() noexcept
	{
		return m_DirectionalLight;
	}

	const DirectionalLight&
		SandboxScene::MainDirectionalLight() const noexcept
	{
		return m_DirectionalLight;
	}

	std::span<SandboxObject>
		SandboxScene::Objects() noexcept
	{
		return m_Objects;
	}

	std::span<const SandboxObject>
		SandboxScene::Objects() const noexcept
	{
		return m_Objects;
	}

	SandboxObject* SandboxScene::FindObject(
		SandboxObjectId objectId) noexcept
	{
		const auto iterator = std::find_if(
			m_Objects.begin(),
			m_Objects.end(),
			[objectId](const SandboxObject& object)
			{
				return object.id == objectId;
			});

		return iterator != m_Objects.end()
			? &*iterator
			: nullptr;
	}

	const SandboxObject* SandboxScene::FindObject(
		SandboxObjectId objectId) const noexcept
	{
		const auto iterator = std::find_if(
			m_Objects.begin(),
			m_Objects.end(),
			[objectId](const SandboxObject& object)
			{
				return object.id == objectId;
			});

		return iterator != m_Objects.end()
			? &*iterator
			: nullptr;
	}

	SceneLightId SandboxScene::AddPointLight()
	{
		if (m_NextLightId == InvalidSceneLightId)
		{
			throw std::overflow_error(
				"Scene light ID space is exhausted.");
		}

		const SceneLightId id = m_NextLightId;

		m_PointLights.push_back(
			PointLight{
				.id = id
			});

		m_NextLightId =
			id == std::numeric_limits<SceneLightId>::max()
			? InvalidSceneLightId
			: id + 1;

		return id;
	}

	SceneLightId SandboxScene::AddSpotLight()
	{
		if (m_NextLightId == InvalidSceneLightId)
		{
			throw std::overflow_error(
				"Scene light ID space is exhausted.");
		}

		const SceneLightId id = m_NextLightId;

		m_SpotLights.push_back(
			SpotLight{
				.id = id
			});

		m_NextLightId =
			id == std::numeric_limits<SceneLightId>::max()
			? InvalidSceneLightId
			: id + 1;

		return id;
	}

	std::span<PointLight>
		SandboxScene::PointLights() noexcept
	{
		return m_PointLights;
	}

	std::span<const PointLight>
		SandboxScene::PointLights() const noexcept
	{
		return m_PointLights;
	}

	const std::optional<SceneCameraData>&
		SandboxScene::SavedCamera() const noexcept
	{
		return m_SavedCamera;
	}

	std::span<SpotLight>
		SandboxScene::SpotLights() noexcept
	{
		return m_SpotLights;
	}

	std::span<const SpotLight>
		SandboxScene::SpotLights() const noexcept
	{
		return m_SpotLights;
	}

	std::optional<CameraValidationError>
		SandboxScene::TrySetSavedCamera(
			const SceneCameraData& camera) noexcept
	{
		if (const auto error = ValidateSceneCamera(camera))
		{
			return error;
		}

		m_SavedCamera = camera;
		return std::nullopt;
	}

	void SandboxScene::ClearSavedCamera() noexcept
	{
		m_SavedCamera.reset();
	}

	const SceneRenderingSettings&
		SandboxScene::RenderingSettings() const noexcept
	{
		return m_RenderingSettings;
	}

	std::optional<RenderingValidationError>
		SandboxScene::TrySetRenderingSettings(
			const SceneRenderingSettings& settings)
	{
		if (const auto error = ValidateSceneRendering(settings))
		{
			return error;
		}

		SceneRenderingSettings replacement = settings;
        using std::swap;
        swap(m_RenderingSettings, replacement);
		return std::nullopt;
	}

    std::span<const SceneCameraBookmark> SandboxScene::CameraBookmarks() const noexcept
    {
        return m_CameraBookmarks;
    }

    void SandboxScene::AddCameraBookmark(std::string name, const SceneCameraData& camera)
    {
        if (const auto error = ValidateCameraBookmarkName(name))
        {
            throw std::invalid_argument(std::string{ error->message });
        }
        if (const auto error = ValidateSceneCamera(camera))
        {
            throw std::invalid_argument("camera." + std::string{ error->property } +
                ": " + std::string{ error->message });
        }
        for (const auto& bookmark : m_CameraBookmarks)
        {
            if (bookmark.name == name)
            {
                throw std::invalid_argument("Bookmark name already exists.");
            }
        }
        m_CameraBookmarks.push_back({ std::move(name), camera });
    }

    void SandboxScene::UpdateCameraBookmark(std::size_t index, const SceneCameraData& camera)
    {
        auto& bookmark = m_CameraBookmarks.at(index);
        if (const auto error = ValidateSceneCamera(camera))
        {
            throw std::invalid_argument("camera." + std::string{ error->property } +
                ": " + std::string{ error->message });
        }
        bookmark.camera = camera;
    }

    void SandboxScene::SelectCameraBookmark(std::size_t index)
    {
        m_SavedCamera = m_CameraBookmarks.at(index).camera;
    }

    void SandboxScene::RemoveCameraBookmark(std::size_t index)
    {
        static_cast<void>(m_CameraBookmarks.at(index));
        m_CameraBookmarks.erase(m_CameraBookmarks.begin() +
            static_cast<std::ptrdiff_t>(index));
        // The active viewpoint is an independent snapshot and remains valid.
    }
}