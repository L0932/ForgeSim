#include "SceneEditHistory.hpp"

#include <utility>
#include <type_traits>
#include <optional>

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

        bool ApplyEdit(SandboxScene& scene, const ObjectRemovalEdit& edit, bool useAfter)
        {
            return useAfter ? scene.RemoveObject(edit.object.id) :
                scene.RestoreObject(edit.index, edit.object, edit.visible);
        }
        bool ApplyEdit(SandboxScene& scene, const PointLightRemovalEdit& edit, bool useAfter)
        {
            return useAfter ? scene.RemovePointLight(edit.light.id) :
                scene.RestorePointLight(edit.index, edit.light, edit.visible);
        }
        bool ApplyEdit(SandboxScene& scene, const SpotLightRemovalEdit& edit, bool useAfter)
        {
            return useAfter ? scene.RemoveSpotLight(edit.light.id) :
                scene.RestoreSpotLight(edit.index, edit.light, edit.visible);
        }
        bool ApplyEdit(SandboxScene& scene, const DirectionalLightEnabledEdit& edit, bool useAfter)
        {
            if (scene.MainDirectionalLight().id != edit.lightId) return false;
            scene.MainDirectionalLight().enabled = useAfter ? edit.after : edit.before;
            return true;
        }

		[[nodiscard]] bool ApplySceneEdit(
			SandboxScene& scene,
			const SceneEdit& edit,
            bool useAfter, SceneSelection* selection = nullptr)
        {
            return std::visit(
                [&scene, useAfter, selection](const auto& concreteEdit)
                {
                    if (!ApplyEdit(scene, concreteEdit, useAfter)) return false;
                    using T = std::decay_t<decltype(concreteEdit)>;
                    if (selection)
                    {
                        if constexpr (std::is_same_v<T, ObjectRemovalEdit>)
                        {
                            if (!useAfter) selection->SelectObject(concreteEdit.object.id);
                            else if (selection->IsObjectSelected(concreteEdit.object.id)) selection->Clear();
                        }
                        else if constexpr (std::is_same_v<T, PointLightRemovalEdit> || std::is_same_v<T, SpotLightRemovalEdit>)
                        {
                            if (!useAfter) selection->SelectLight(concreteEdit.light.id);
                            else if (selection->IsLightSelected(concreteEdit.light.id)) selection->Clear();
                        }
                        else if constexpr (std::is_same_v<T, DirectionalLightEnabledEdit>)
                        {
                            if (!useAfter) selection->SelectLight(concreteEdit.lightId);
                            else if (selection->IsLightSelected(concreteEdit.lightId)) selection->Clear();
                        }
                    }
                    return true;
                },
				edit);
		}
	}

    bool CanDeleteSceneSelection(const SandboxScene& scene, const SceneSelection& selection)
    {
        if (const auto id = selection.SelectedObjectId()) return scene.FindObject(*id) != nullptr;
        if (const auto id = selection.SelectedLightId())
        {
            if (*id == scene.MainDirectionalLight().id) return scene.MainDirectionalLight().enabled;
            for (const auto& light : scene.PointLights()) if (light.id == *id) return true;
            for (const auto& light : scene.SpotLights()) if (light.id == *id) return true;
        }
        return false;
    }

    bool SceneEditHistory::DeleteSelection(SandboxScene& scene, SceneSelection& selection)
    {
        if (!CanDeleteSceneSelection(scene, selection)) return false;
        std::optional<SceneEdit> edit;
        if (const auto id = selection.SelectedObjectId())
        {
            const auto objects = scene.Objects();
            for (std::size_t i = 0; i < objects.size(); ++i)
                if (objects[i].id == *id) edit = ObjectRemovalEdit{objects[i], i, scene.IsObjectVisible(*id)};
        }
        else if (const auto lightId = selection.SelectedLightId())
        {
            if (*lightId == scene.MainDirectionalLight().id)
                edit = DirectionalLightEnabledEdit{*lightId, scene.MainDirectionalLight().enabled, false};
            const auto points = scene.PointLights();
            for (std::size_t i = 0; i < points.size(); ++i)
                if (points[i].id == *lightId) edit = PointLightRemovalEdit{points[i], i, scene.IsLightVisible(*lightId)};
            const auto spots = scene.SpotLights();
            for (std::size_t i = 0; i < spots.size(); ++i)
                if (spots[i].id == *lightId) edit = SpotLightRemovalEdit{spots[i], i, scene.IsLightVisible(*lightId)};
        }
        if (!edit) return false;
        static_assert(std::is_nothrow_move_constructible_v<SceneEdit>);
        m_Edits.reserve(m_Position + 1);
        if (!ApplySceneEdit(scene, *edit, true, &selection)) return false;
        RecordApplied(std::move(*edit));
        return true;
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

    bool SceneEditHistory::Undo(SandboxScene& scene, SceneSelection* selection)
    {
        while (CanUndo())
        {
            const auto index = m_Position - 1;
            // Restoration can allocate. Advance history only after it succeeds.
            const bool applied = ApplySceneEdit(scene, m_Edits[index], false, selection);
            m_Position = index;
            if (applied) return true;
        }
        return false;
    }

    bool SceneEditHistory::Redo(SandboxScene& scene, SceneSelection* selection)
    {
        while (CanRedo())
        {
            const bool applied = ApplySceneEdit(scene, m_Edits[m_Position], true, selection);
            ++m_Position;
            if (applied) return true;
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