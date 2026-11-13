#pragma once

#include "SceneEditHistory.hpp"
#include "SceneSelection.hpp"
#include "ViewportTransformGizmo.hpp"

#include <optional>

namespace ForgeSim::Sandbox
{
	struct SceneTransformEditorContext
	{
		SandboxScene& scene;
		const SceneSelection& selection;

		const glm::mat4& view;
		const glm::mat4& projection;

		glm::vec2 viewportPosition{ 0.0f };
		glm::vec2 viewportSize{ 0.0f };
		bool viewportHovered = false;

		GizmoOperation operation = GizmoOperation::Translate;
		GizmoSpace space = GizmoSpace::World;

		bool showLights = true;
		bool navigationActive = false;
	};

	class SceneTransformEditor final
	{
	public:
		void BeginFrame();

        void RecordObjectEdits(ObjectBatchEdit batch);
        [[nodiscard]] bool SetNodeOverride(SandboxObject& object, std::size_t nodeIndex,
            const std::optional<Transform>& value);
        [[nodiscard]] bool SetSelectedObjectPivots(SandboxScene& scene, const SceneSelection& selection,
            const Renderer::AxisAlignedBoundingBox& fallbackBounds, bool boundsCenter);
        [[nodiscard]] bool SetSelectionVisible(SandboxScene& scene, const SceneSelection& selection, bool visible);

        // Changes only the pivot, preserving placement and recording undo/redo.
        [[nodiscard]] bool SetObjectPivot(SandboxObject& object, const glm::vec3& pivot);

		void Update(
			const SceneTransformEditorContext& context);

		[[nodiscard]] bool IsActive() const noexcept;
		[[nodiscard]] bool SuppressesPicking() const noexcept;

        [[nodiscard]] bool CanUndo() const noexcept;
        [[nodiscard]] bool CanRedo() const noexcept;

		[[nodiscard]] bool Undo(SandboxScene& scene, SceneSelection* selection = nullptr);
		[[nodiscard]] bool Redo(SandboxScene& scene, SceneSelection* selection = nullptr);

        [[nodiscard]] bool DuplicateSelection(SandboxScene& scene, SceneSelection& selection);
        [[nodiscard]] bool CreateCube(SandboxScene& scene, SceneSelection& selection, const glm::vec3& position);
        [[nodiscard]] bool CreateLight(SandboxScene& scene, SceneSelection& selection,
            SceneLightKind type, const glm::vec3& position, const glm::vec3& direction, float range);
        [[nodiscard]] bool DeleteSelection(SandboxScene& scene, SceneSelection& selection);

		// Call when replacing the scene, outside an active drag.
		void ClearHistory();

	private:
		template<typename Target>
		void EditTarget(
			Target& target,
			const SceneTransformEditorContext& context);

		[[nodiscard]] GizmoInteraction DrawGizmo(
			glm::mat4& matrix,
			const SceneTransformEditorContext& context,
			bool operationSupported);

		void FinishEdit();
        void EditObjectGroup(const SceneTransformEditorContext& context);
        std::optional<ObjectBatchEdit> m_PendingObjectEdits;
        SceneSelection m_DragSelection;

		ViewportTransformGizmo m_Gizmo;
		GizmoInteraction m_Interaction;

		SceneEditHistory m_History;
		std::optional<SceneTransformEdit> m_PendingEdit;

		bool m_WasActive = false;
	};
}