#include "SandboxScene.hpp"
#include "SceneAssetPath.hpp"

#include <stdexcept>
#include <utility>
#include <algorithm>
#include <limits>
#include <type_traits>

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

    SandboxObjectId SandboxScene::AddModelObject(std::string name, std::string modelPath)
    {
        if (!IsValidSceneAssetPath(modelPath))
            throw std::invalid_argument("Imported model needs a portable project-relative path.");
        SandboxObjectId highest = m_LastObjectId;
        for (const auto& object : m_Objects) highest = std::max(highest, object.id);
        if (highest == std::numeric_limits<SandboxObjectId>::max())
            throw std::overflow_error("No object IDs remain.");
        const SandboxObjectId id = highest + 1;
        m_Objects.push_back(SandboxObject{.id = id, .transform = {},
            .name = std::move(name), .modelPath = std::move(modelPath)});
        m_LastObjectId = id;
        return id;
    }

    namespace
    {
        template<class T, class Id>
        bool RemoveEntity(std::vector<T>& entities, std::unordered_set<Id>& hidden, Id id)
        {
            const auto found = std::find_if(entities.begin(), entities.end(),
                [id](const T& entity) { return entity.id == id; });
            if (found == entities.end()) return false;
            entities.erase(found);
            hidden.erase(id);
            return true;
        }
        template<class T, class Id>
        bool RestoreEntity(std::vector<T>& entities, std::unordered_set<Id>& hidden,
            std::size_t index, const T& entity, bool visible)
        {
            if (index > entities.size() || std::any_of(entities.begin(), entities.end(),
                [&](const T& existing) { return existing.id == entity.id; })) return false;
            static_assert(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>);
            T copy = entity;
            entities.reserve(entities.size() + 1);
            // Finish allocating before the no-throw moves that insert the entity.
            if (!visible) hidden.insert(entity.id);
            entities.insert(entities.begin() + static_cast<std::ptrdiff_t>(index), std::move(copy));
            if (visible) hidden.erase(entity.id);
            return true;
        }
        bool LightIdExists(const SandboxScene& scene, SceneLightId id)
        {
            if (scene.MainDirectionalLight().id == id) return true;
            for (const auto& light : scene.PointLights()) if (light.id == id) return true;
            for (const auto& light : scene.SpotLights()) if (light.id == id) return true;
            return false;
        }
    }
    SceneLightId SandboxScene::AddDirectionalLight()
    {
        if (HasDirectionalLight()) throw std::invalid_argument("The scene already has a directional light.");
        if (m_NextLightId == InvalidSceneLightId) throw std::overflow_error("Scene light ID space is exhausted.");
        const auto id = m_NextLightId;
        m_DirectionalLight = DirectionalLight{.id = id};
        m_NextLightId = id == std::numeric_limits<SceneLightId>::max() ? InvalidSceneLightId : id + 1;
        return id;
    }
    bool SandboxScene::RemoveDirectionalLight(SceneLightId id)
    {
        if (!HasDirectionalLight() || m_DirectionalLight.id != id) return false;
        m_HiddenLights.erase(id);
        m_DirectionalLight = DirectionalLight{.id = InvalidSceneLightId, .enabled = false};
        return true;
    }
    bool SandboxScene::RestoreDirectionalLight(const DirectionalLight& light, bool visible)
    {
        if (HasDirectionalLight() || light.id == InvalidSceneLightId || LightIdExists(*this, light.id)) return false;
        if (!visible) m_HiddenLights.insert(light.id);
        m_DirectionalLight = light;
        if (visible) m_HiddenLights.erase(light.id);
        if (m_NextLightId != InvalidSceneLightId && light.id >= m_NextLightId)
            m_NextLightId = light.id == std::numeric_limits<SceneLightId>::max() ? InvalidSceneLightId : light.id + 1;
        return true;
    }
    bool SandboxScene::RemoveObject(SandboxObjectId id)
    { return RemoveEntity(m_Objects, m_HiddenObjects, id); }
    bool SandboxScene::RemovePointLight(SceneLightId id)
    { return RemoveEntity(m_PointLights, m_HiddenLights, id); }
    bool SandboxScene::RemoveSpotLight(SceneLightId id)
    { return RemoveEntity(m_SpotLights, m_HiddenLights, id); }
    bool SandboxScene::RestoreObject(std::size_t index, const SandboxObject& object, bool visible)
    {
        if (object.id == 0) return false;
        if (!RestoreEntity(m_Objects, m_HiddenObjects, index, object, visible)) return false;
        m_LastObjectId = std::max(m_LastObjectId, object.id);
        return true;
    }
    bool SandboxScene::RestorePointLight(std::size_t index, const PointLight& light, bool visible)
    {
        if (light.id == InvalidSceneLightId || LightIdExists(*this, light.id) || !RestoreEntity(m_PointLights, m_HiddenLights, index, light, visible)) return false;
        if (m_NextLightId != InvalidSceneLightId && light.id >= m_NextLightId)
            m_NextLightId = light.id == std::numeric_limits<SceneLightId>::max() ? InvalidSceneLightId : light.id + 1;
        return true;
    }
    bool SandboxScene::RestoreSpotLight(std::size_t index, const SpotLight& light, bool visible)
    {
        if (light.id == InvalidSceneLightId || LightIdExists(*this, light.id) || !RestoreEntity(m_SpotLights, m_HiddenLights, index, light, visible)) return false;
        if (m_NextLightId != InvalidSceneLightId && light.id >= m_NextLightId)
            m_NextLightId = light.id == std::numeric_limits<SceneLightId>::max() ? InvalidSceneLightId : light.id + 1;
        return true;
    }

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

    std::optional<CameraValidationError> SandboxScene::TrySetCameraMovementSpeed(std::optional<float> speed) noexcept
    {
        if (speed)
            if (const auto error = ValidateCameraMovementSpeed(*speed)) return error;
        m_CameraMovementSpeed = speed;
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