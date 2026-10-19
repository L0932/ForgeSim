#pragma once

#include "SandboxScene.hpp"
#include "Transform.hpp"

#include <cstddef>
#include <variant>
#include <vector>

namespace ForgeSim::Sandbox
{
	struct ObjectTransformEdit
	{
		SandboxObjectId objectId;
		Transform before;
		Transform after;
	};

	struct PointLightPositionEdit
	{
		SceneLightId lightId;
		glm::vec3 before;
		glm::vec3 after;
	};

	struct SpotLightPose
	{
		glm::vec3 position;
		glm::vec3 emissionDirection;
	};

	struct SpotLightPoseEdit
	{
		SceneLightId lightId;
		SpotLightPose before;
		SpotLightPose after;
	};

	struct DirectionalLightDirectionEdit
	{
		SceneLightId lightId;
		glm::vec3 before;
		glm::vec3 after;
	};

	using SceneEdit = std::variant<
		ObjectTransformEdit,
		PointLightPositionEdit,
		SpotLightPoseEdit,
		DirectionalLightDirectionEdit>;

	class SceneEditHistory final
	{
	public:
		// The caller has already applied the edit to the scene.
		// Recording a new edit discards the redo branch.
		void RecordApplied(SceneEdit edit);

		[[nodiscard]] bool Undo(SandboxScene& scene);
		[[nodiscard]] bool Redo(SandboxScene& scene);

		[[nodiscard]] bool CanUndo() const noexcept;
		[[nodiscard]] bool CanRedo() const noexcept;

		// Call when replacing the scene
		void Clear() noexcept;

	private:
		std::vector<SceneEdit> m_Edits;

		// Entries before this index are applied.
		// Entries at or after it are available for redo.
		std::size_t m_Position = 0;
	};
}