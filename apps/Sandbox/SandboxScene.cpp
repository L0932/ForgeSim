#include "SandboxScene.hpp"

#include <algorithm>

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
					.scale = glm::vec3{ 300.0f }
				}
			}
		}
	{}

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
}