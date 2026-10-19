#include "LightGizmoTransform.hpp"
#include "SceneTransformEditor.hpp"
#include "GizmoTransform.hpp"

#include <type_traits>
#include <utility>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] Transform ReadTransform(
			const SandboxObject& object)
		{
			return object.transform;
		}

		[[nodiscard]] Transform ReadTransform(
			const PointLight& light)
		{
			Transform transform;
			transform.position = light.position;
			return transform;
		}

		[[nodiscard]] bool SupportsOperation(
			const SandboxObject&,
			GizmoOperation operation)
		{
			return operation == GizmoOperation::Translate ||
				operation == GizmoOperation::Rotate ||
				operation == GizmoOperation::Scale;
		}

		[[nodiscard]] bool SupportsOperation(
			const DirectionalLight&,
			GizmoOperation operation)
		{
			return operation == GizmoOperation::Translate ||
				operation == GizmoOperation::Rotate;
		}

		[[nodiscard]] bool SupportsOperation(
			const PointLight&,
			GizmoOperation operation)
		{
			return operation == GizmoOperation::Translate;
		}

		[[nodiscard]] bool SupportsOperation(
			const SpotLight&,
			GizmoOperation operation)
		{
			return operation == GizmoOperation::Translate ||
				operation == GizmoOperation::Rotate;
		}

		template<typename Target>
		[[nodiscard]] bool TryReadTransform(
			const Target& target,
			Transform& transform)
		{
			transform = ReadTransform(target);
			return true;
		}

		[[nodiscard]] bool TryReadTransform(
			const DirectionalLight& light,
			Transform& transform)
		{
			return TryCreateLightGizmoTransform(
				light.position,
				-light.directionToLight,
				transform);
		}

		[[nodiscard]] bool TryReadTransform(
			const SpotLight& light,
			Transform& transform)
		{
			return TryCreateLightGizmoTransform(
				light.position,
				light.emissionDirection,
				transform);
		}

		void WriteTransform(
			SandboxObject& object,
			const Transform& transform,
			GizmoOperation)
		{
			object.transform = transform;
		}

		void WriteTransform(
			DirectionalLight& light,
			const Transform& transform,
			GizmoOperation operation)
		{
			if (operation == GizmoOperation::Translate)
			{
				light.position = transform.position;
			}
			else if (operation == GizmoOperation::Rotate)
			{
				light.directionToLight =
					-LightGizmoEmissionDirection(transform);
			}
		}

		void WriteTransform(
			PointLight& light,
			const Transform& transform,
			GizmoOperation)
		{
			light.position = transform.position;
		}

		void WriteTransform(
			SpotLight& light,
			const Transform& transform,
			GizmoOperation operation)
		{
			if (operation == GizmoOperation::Translate)
			{
				light.position = transform.position;
			}
			else if (operation == GizmoOperation::Rotate)
			{
				light.emissionDirection =
					LightGizmoEmissionDirection(transform);
			}
		}

		[[nodiscard]] SceneEdit MakeEdit(
			const SandboxObject& object,
			const Transform& before,
			const Transform& after,
			GizmoOperation)
		{
			return ObjectTransformEdit{
				.objectId = object.id,
				.before = before,
				.after = after
			};
		}

		[[nodiscard]] SceneEdit MakeEdit(
			const DirectionalLight& light,
			const Transform& before,
			const Transform& after,
			GizmoOperation operation)
		{
			if (operation == GizmoOperation::Translate)
			{
				return DirectionalLightPositionEdit{
					.lightId = light.id,
					.before = before.position,
					.after = after.position
				};
			}

			return DirectionalLightDirectionEdit{
				.lightId = light.id,
				.before = light.directionToLight,
				.after = -LightGizmoEmissionDirection(after)
			};
		}

		[[nodiscard]] SceneEdit MakeEdit(
			const PointLight& light,
			const Transform& before,
			const Transform& after,
			GizmoOperation)
		{
			return PointLightPositionEdit{
				.lightId = light.id,
				.before = before.position,
				.after = after.position
			};
		}

		[[nodiscard]] SceneEdit MakeEdit(
			const SpotLight& light,
			const Transform& before,
			const Transform& after,
			GizmoOperation operation)
		{
			return SpotLightPoseEdit{
				.lightId = light.id,
				.before = {
					.position = before.position,
					.emissionDirection =
						light.emissionDirection
				},
				.after = {
					.position =
						operation == GizmoOperation::Translate
						? after.position
						: before.position,
					.emissionDirection =
						operation == GizmoOperation::Rotate
						? LightGizmoEmissionDirection(after)
						: light.emissionDirection
				}
			};
		}
	}

	void SceneTransformEditor::BeginFrame()
	{
		m_Gizmo.BeginFrame();
	}

	template<typename Target>
	void SceneTransformEditor::EditTarget(
		Target& target,
		const SceneTransformEditorContext& context)
	{
		Transform before;

		if (!TryReadTransform(target, before))
		{
			return;
		}

		Transform after = before;
		glm::mat4 matrix = before.ModelMatrix();

		const bool operationSupported =
			SupportsOperation(target, context.operation);

		m_Interaction = DrawGizmo(
			matrix,
			context,
			operationSupported);

		if (!operationSupported ||
			!m_Interaction.changed ||
			!TryApplyGizmoTransform(
				after,
				matrix,
				context.operation))
		{
			return;
		}

		SceneEdit edit = MakeEdit(
			target,
			before,
			after,
			context.operation);

		if (!m_PendingEdit)
		{
			m_PendingEdit = std::move(edit);
		}
		else
		{
			// Preserve the original state from the start
			// of the drag and update only its final state.
			std::visit(
				[this](const auto& latest)
				{
					using Edit =
						std::decay_t<decltype(latest)>;

					std::get<Edit>(*m_PendingEdit).after =
						latest.after;
				},
				edit);
		}

		WriteTransform(target, after, context.operation);
	}

	GizmoInteraction SceneTransformEditor::DrawGizmo(
		glm::mat4& matrix,
		const SceneTransformEditorContext& context,
		bool operationSupported)
	{
		return m_Gizmo.DrawInMainViewport(
			ViewportTransformGizmoContext{
				.view = context.view,
				.projection = context.projection,
				.transform = matrix,
				.operation = context.operation,
				.space = context.space,
				.enabled =
					operationSupported &&
					(m_WasActive || !context.navigationActive)
			},
			m_WasActive);
	}

	void SceneTransformEditor::Update(
		const SceneTransformEditorContext& context)
	{
		m_WasActive = m_Interaction.active;
		m_Interaction = {};

		if (m_PendingEdit)
		{
			// Continue editing the original target even
			// if selection changes through another UI.
			std::visit(
				[this, &context](const auto& edit)
				{
					using Edit =
						std::decay_t<decltype(edit)>;

					if constexpr (
						std::is_same_v<
						Edit,
						ObjectTransformEdit>)
					{
						if (SandboxObject* object =
							context.scene.FindObject(
								edit.objectId))
						{
							EditTarget(*object, context);
						}
					}
					else if constexpr (
						std::is_same_v<
						Edit,
						PointLightPositionEdit>)
					{
						if (context.showLights &&
							context.operation ==
							GizmoOperation::Translate)
						{
							for (PointLight& light :
								context.scene.PointLights())
							{
								if (light.id == edit.lightId)
								{
									EditTarget(light, context);
									break;
								}
							}
						}
					}
					else if constexpr (
						std::is_same_v<
						Edit,
						SpotLightPoseEdit>)
					{
						if (context.showLights &&
							(context.operation ==
								GizmoOperation::Translate ||
								context.operation ==
								GizmoOperation::Rotate))
						{
							for (SpotLight& light :
								context.scene.SpotLights())
							{
								if (light.id == edit.lightId)
								{
									EditTarget(light, context);
									break;
								}
							}
						}
					}
					else if constexpr (
						std::is_same_v<Edit, DirectionalLightDirectionEdit> ||
						std::is_same_v<Edit, DirectionalLightPositionEdit>)
					{
						if (context.showLights &&
							(context.operation == GizmoOperation::Translate ||
								context.operation == GizmoOperation::Rotate))
						{
							DirectionalLight& light =
								context.scene.MainDirectionalLight();

							if (light.id == edit.lightId)
							{
								EditTarget(light, context);
							}
						}
					}
				},
				*m_PendingEdit);
		}
		else if (const auto objectId =
			context.selection.SelectedObjectId())
		{
			if (SandboxObject* object =
				context.scene.FindObject(*objectId))
			{
				EditTarget(*object, context);
			}
		}
		else if (const auto lightId =
			context.selection.SelectedLightId();
			lightId && context.showLights)
		{
			DirectionalLight& directional =
				context.scene.MainDirectionalLight();

			if (directional.id == *lightId)
			{
				EditTarget(directional, context);
			}
			else
			{
				bool found = false;

				for (PointLight& light : context.scene.PointLights())
				{
					if (light.id == *lightId)
					{
						EditTarget(light, context);
						found = true;
						break;
					}
				}

				if (!found)
				{
					for (SpotLight& light : context.scene.SpotLights())
					{
						if (light.id == *lightId)
						{
							EditTarget(light, context);
							break;
						}
					}
				}
			}
		}

		if (!m_Interaction.active)
		{
			FinishEdit();
		}
	}

	void SceneTransformEditor::FinishEdit()
	{
		if (m_PendingEdit)
		{
			m_History.RecordApplied(
				std::move(*m_PendingEdit));

			m_PendingEdit.reset();
		}
	}

	bool SceneTransformEditor::IsActive() const noexcept
	{
		return m_Interaction.active;
	}

	bool SceneTransformEditor::SuppressesPicking() const noexcept
	{
		return m_WasActive ||
			m_Interaction.hovered ||
			m_Interaction.active;
	}

	bool SceneTransformEditor::Undo(SandboxScene& scene)
	{
		if (m_WasActive || IsActive())
		{
			return false;
		}

		return m_History.Undo(scene);
	}

	bool SceneTransformEditor::Redo(SandboxScene& scene)
	{
		if (m_WasActive || IsActive())
		{
			return false;
		}

		return m_History.Redo(scene);
	}

	void SceneTransformEditor::ClearHistory()
	{
		m_PendingEdit.reset();
		m_History.Clear();
	}
}