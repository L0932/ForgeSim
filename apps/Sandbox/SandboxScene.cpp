#include "SandboxScene.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

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
		if (m_NextLightId ==
			std::numeric_limits<SceneLightId>::max())
		{
			throw std::overflow_error(
				"Scene light ID space is exhausted.");
		}

		const SceneLightId id = m_NextLightId;

		m_PointLights.push_back(
			PointLight{
				.id = id
			});

		++m_NextLightId;

		return id;
	}

	SceneLightId SandboxScene::AddSpotLight()
	{
		if (m_NextLightId ==
			std::numeric_limits<SceneLightId>::max())
		{
			throw std::overflow_error(
				"Scene light ID space is exhausted.");
		}

		const SceneLightId id = m_NextLightId;

		m_SpotLights.push_back(
			SpotLight{
				.id = id
			});

		++m_NextLightId;

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
}