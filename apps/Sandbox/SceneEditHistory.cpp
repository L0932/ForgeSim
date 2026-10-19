#include "SceneEditHistory.hpp"

#include <utility>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] bool ApplyEdit(
			SandboxScene& scene,
			const ObjectTransformEdit& edit,
			bool useAfter)
		{
			SandboxObject* object =
				scene.FindObject(edit.objectId);

			if (object == nullptr)
			{
				return false;
			}

			object->transform =
				useAfter ? edit.after : edit.before;

			return true;
		}

		[[nodiscard]] bool ApplyEdit(
			SandboxScene& scene,
			const PointLightPositionEdit& edit,
			bool useAfter)
		{
			for (PointLight& light : scene.PointLights())
			{
				if (light.id == edit.lightId)
				{
					light.position =
						useAfter ? edit.after : edit.before;

					return true;
				}
			}

			return false;
		}

		[[nodiscard]] bool ApplyEdit(
			SandboxScene& scene,
			const SpotLightPoseEdit& edit,
			bool useAfter)
		{
			for (SpotLight& light : scene.SpotLights())
			{
				if (light.id == edit.lightId)
				{
					const SpotLightPose& pose =
						useAfter ? edit.after : edit.before;

					light.position = pose.position;
					light.emissionDirection = pose.emissionDirection;

					return true;
				}
			}

			return false;
		}

		[[nodiscard]] bool ApplyEdit(
			SandboxScene& scene,
			const DirectionalLightDirectionEdit& edit,
			bool useAfter)
		{
			DirectionalLight& light =
				scene.MainDirectionalLight();

			if (light.id != edit.lightId)
			{
				return false;
			}

			light.directionToLight =
				useAfter ? edit.after : edit.before;

			return true;
		}

		[[nodiscard]] bool ApplyEdit(
			SandboxScene& scene,
			const DirectionalLightPositionEdit& edit,
			bool useAfter)
		{
			DirectionalLight& light =
				scene.MainDirectionalLight();

			if (light.id != edit.lightId)
			{
				return false;
			}

			light.position =
				useAfter ? edit.after : edit.before;

			return true;
		}

		[[nodiscard]] bool ApplySceneEdit(
			SandboxScene& scene,
			const SceneEdit& edit,
			bool useAfter)
		{
			return std::visit(
				[&scene, useAfter](const auto& concreteEdit)
				{
					return ApplyEdit(
						scene,
						concreteEdit,
						useAfter);
				},
				edit);
		}
	}

	void SceneEditHistory::RecordApplied(SceneEdit edit)
	{
		// Reserve before discarding redo entries so allocation
		// failure cannot remove the existing redo history.
		m_Edits.reserve(m_Position + 1);
		m_Edits.resize(m_Position);
		m_Edits.push_back(std::move(edit));

		m_Position = m_Edits.size();
	}

	bool SceneEditHistory::Undo(SandboxScene& scene)
	{
		while (CanUndo())
		{
			--m_Position;

			if (ApplySceneEdit(
				scene,
				m_Edits[m_Position],
				false))
			{
				return true;
			}
		}

		return false;
	}

	bool SceneEditHistory::Redo(SandboxScene& scene)
	{
		while (CanRedo())
		{
			const std::size_t index = m_Position;
			++m_Position;

			if (ApplySceneEdit(
				scene,
				m_Edits[index],
				true))
			{
				return true;
			}
		}

		return false;
	}

	bool SceneEditHistory::CanUndo() const noexcept
	{
		return m_Position > 0;
	}

	bool SceneEditHistory::CanRedo() const noexcept
	{
		return m_Position < m_Edits.size();
	}

	void SceneEditHistory::Clear() noexcept
	{
		m_Edits.clear();
		m_Position = 0;
	}
}