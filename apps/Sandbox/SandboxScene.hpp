#pragma once

#include "SandboxObject.hpp"
#include "SceneLight.hpp"

#include <span>
#include <string>
#include <vector>

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

		[[nodiscard]] SceneLightId AddPointLight();
		[[nodiscard]] SceneLightId AddSpotLight();

		[[nodiscard]] std::span<PointLight>
			PointLights() noexcept;

		[[nodiscard]] std::span<const PointLight>
			PointLights() const noexcept;

		[[nodiscard]] std::span<SpotLight>
			SpotLights() noexcept;

		[[nodiscard]] std::span<const SpotLight>
			SpotLights() const noexcept;

	private:
		std::vector<SandboxObject> m_Objects;
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
	};
}