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

		void Update(
			const SceneTransformEditorContext& context);

		[[nodiscard]] bool IsActive() const noexcept;
		[[nodiscard]] bool SuppressesPicking() const noexcept;

		[[nodiscard]] bool Undo(SandboxScene& scene);
		[[nodiscard]] bool Redo(SandboxScene& scene);

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

		ViewportTransformGizmo m_Gizmo;
		GizmoInteraction m_Interaction;

		SceneEditHistory m_History;
		std::optional<SceneEdit> m_PendingEdit;

		bool m_WasActive = false;
	};
}