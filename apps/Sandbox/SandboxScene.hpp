#pragma once

#include "SandboxObject.hpp"
#include "SceneCameraData.hpp"
#include "SceneCameraValidation.hpp"
#include "SceneRenderingValidation.hpp"
#include "SceneLight.hpp"

#include <optional>
#include <span>
#include <string>
#include <vector>
#include <unordered_set>

namespace ForgeSim::Sandbox
{
	struct SceneDocument;

	class SandboxScene final
	{
	public:
		SandboxScene();

		explicit SandboxScene(const SceneDocument& document);

		[[nodiscard]] SceneDocument ToDocument() const;

		[[nodiscard]] const std::string& Name() const noexcept;

		[[nodiscard]] DirectionalLight&
			MainDirectionalLight() noexcept;

		[[nodiscard]] const DirectionalLight&
			MainDirectionalLight() const noexcept;

		[[nodiscard]] std::span<SandboxObject>
			Objects() noexcept;

		[[nodiscard]] std::span<const SandboxObject>
			Objects() const noexcept;

		[[nodiscard]] SandboxObject* FindObject(
			SandboxObjectId objectId) noexcept;

		[[nodiscard]] const SandboxObject* FindObject(
			SandboxObjectId objectId) const noexcept;

        [[nodiscard]] bool HasAllocatedObjectIds() const noexcept { return m_LastObjectId != 0; }
        // Restore trusted history snapshots without reassigning IDs.
        [[nodiscard]] bool RemoveObject(SandboxObjectId id);
        [[nodiscard]] bool RemovePointLight(SceneLightId id);
        [[nodiscard]] bool RemoveSpotLight(SceneLightId id);
        [[nodiscard]] bool RestoreObject(std::size_t index, const SandboxObject& object, bool visible);
        [[nodiscard]] bool RestorePointLight(std::size_t index, const PointLight& light, bool visible);
        [[nodiscard]] bool RestoreSpotLight(std::size_t index, const SpotLight& light, bool visible);

        [[nodiscard]] SandboxObjectId AddModelObject(std::string name, std::string modelPath);

		[[nodiscard]] SceneLightId AddPointLight();
		[[nodiscard]] SceneLightId AddSpotLight();

		[[nodiscard]] std::span<PointLight>
			PointLights() noexcept;

		[[nodiscard]] std::span<const PointLight>
			PointLights() const noexcept;

		[[nodiscard]] const std::optional<SceneCameraData>&
			SavedCamera() const noexcept;

		[[nodiscard]] std::span<SpotLight>
			SpotLights() noexcept;

		[[nodiscard]] std::span<const SpotLight>
			SpotLights() const noexcept;

		[[nodiscard]] std::optional<CameraValidationError>
			TrySetSavedCamera(const SceneCameraData& camera) noexcept;

		[[nodiscard]] const SceneRenderingSettings&
			RenderingSettings() const noexcept;

		[[nodiscard]] std::optional<RenderingValidationError>
			TrySetRenderingSettings(
				const SceneRenderingSettings& settings);

        [[nodiscard]] std::span<const SceneCameraBookmark>
            CameraBookmarks() const noexcept;

        void AddCameraBookmark(std::string name, const SceneCameraData& camera);
        void UpdateCameraBookmark(std::size_t index, const SceneCameraData& camera);
        void SelectCameraBookmark(std::size_t index);
        void RemoveCameraBookmark(std::size_t index);

        [[nodiscard]] const std::optional<float>& CameraMovementSpeed() const noexcept
        { return m_CameraMovementSpeed; }
        [[nodiscard]] std::optional<CameraValidationError>
        TrySetCameraMovementSpeed(std::optional<float> speed) noexcept;

		void ClearSavedCamera() noexcept;

        [[nodiscard]] bool IsObjectVisible(SandboxObjectId id) const noexcept
        { return !m_HiddenObjects.contains(id); }
        [[nodiscard]] bool IsLightVisible(SceneLightId id) const noexcept
        { return !m_HiddenLights.contains(id); }
        [[nodiscard]] bool HasHiddenObjects() const noexcept { return !m_HiddenObjects.empty(); }
        [[nodiscard]] bool HasHiddenLights() const noexcept { return !m_HiddenLights.empty(); }
        void SetObjectVisible(SandboxObjectId id, bool visible);
        void SetLightVisible(SceneLightId id, bool visible);
        void ShowAllObjects() noexcept { m_HiddenObjects.clear(); }
        void ShowAllLights() noexcept { m_HiddenLights.clear(); }

	private:
		std::vector<SandboxObject> m_Objects;
        SandboxObjectId m_LastObjectId = 1;
        std::unordered_set<SandboxObjectId> m_HiddenObjects;
        std::unordered_set<SceneLightId> m_HiddenLights;
		std::string m_Name = "Untitled";

		DirectionalLight m_DirectionalLight{
			.id = 1,
			.enabled = true,
			.directionToLight = {
			   -0.5f,
				1.0f,
				0.3f
			},
			.color = {
				1.0f,
				0.95f,
				0.9f
			},
			.intensity = 3.0f
		};

		std::vector<PointLight> m_PointLights;
		std::vector<SpotLight> m_SpotLights;

		SceneLightId m_NextLightId = 2;

		std::optional<SceneCameraData> m_SavedCamera;
        std::optional<float> m_CameraMovementSpeed;
        std::vector<SceneCameraBookmark> m_CameraBookmarks;

		SceneRenderingSettings m_RenderingSettings;
	};
}