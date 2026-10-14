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
					.position =
						glm::vec3{ -1.25f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(25.0f),
						glm::radians(35.0f),
						0.0f
					},
					.scale = glm::vec3{ 10.0f }
				}
			},
			SandboxObject{
				.id = 2,
				.transform = Transform{
					.position =
						glm::vec3{ 0.0f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(15.0f),
						glm::radians(-20.0f),
						0.0f
					},
					.scale = glm::vec3{ 1.0f }
				}
			},
			SandboxObject{
				.id = 3,
				.transform = Transform{
					.position =
						glm::vec3{ 1.25f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(-15.0f),
						glm::radians(30.0f),
						0.0f
					},
					.scale = glm::vec3{ 0.6f }
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