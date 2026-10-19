#include "LightGizmoTransform.hpp"
#include "SceneTransformEditor.hpp"
#include "GizmoTransform.hpp"

#include <cmath>
#include <type_traits>
#include <utility>

#include <glm/geometric.hpp>
#include <glm/gtx/euler_angles.hpp>

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

		template<typename Target>
		[[nodiscard]] bool TryReadTransform(
			const Target& target,
			Transform& transform)
		{
			transform = ReadTransform(target);
			return true;
		}

		[[nodiscard]] bool TryReadLightTransform(
			const glm::vec3& position,
			const glm::vec3& direction,
			Transform& transform)
		{
			const float lengthSquared =
				glm::dot(direction, direction);

			if (!std::isfinite(lengthSquared) ||
				lengthSquared < 1e-12f)
			{
				return false;
			}

			const glm::vec3 forward =
				direction / std::sqrt(lengthSquared);

			// Choose a reference axis that isn't parallel
			// to the emission direction.
			const glm::vec3 referenceUp =
				std::abs(forward.y) < 0.99f
				? glm::vec3{ 0.0f, 1.0f, 0.0f }
			: glm::vec3{ 1.0f, 0.0f, 0.0f };

			const glm::vec3 right =
				glm::normalize(
					glm::cross(forward, referenceUp));

			const glm::vec3 up =
				glm::cross(right, forward);

			glm::mat4 rotation{ 1.0f };
			rotation[0] = glm::vec4{ right, 0.0f };
			rotation[1] = glm::vec4{ up, 0.0f };
			rotation[2] = glm::vec4{ -forward, 0.0f };

			transform = Transform{};
			transform.position = position;

			glm::extractEulerAngleXYZ(
				rotation,
				transform.rotationRadians.x,
				transform.rotationRadians.y,
				transform.rotationRadians.z);

			return true;
		}

		[[nodiscard]] bool TryReadTransform(
			const SpotLight& light,
			Transform& transform)
		{
			return TryReadLightTransform(
				light.position,
				light.emissionDirection,
				transform);
		}

		[[nodiscard]] bool TryReadTransform(
			const DirectionalLight& light,
			Transform& transform)
		{
			// Local -Z follows emission, which is opposite
			// to the scene's directionToLight convention.
			return TryReadLightTransform(
				glm::vec3{ 0.0f },
				-light.directionToLight,
				transform);
		}

		[[nodiscard]] glm::vec3 EmissionDirection(
			const Transform& transform)
		{
			return glm::normalize(
				-glm::vec3{ transform.ModelMatrix()[2] });
		}

		void WriteTransform(
			SandboxObject& object,
			const Transform& transform,
			GizmoOperation)
		{
			object.transform = transform;
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
					EmissionDirection(transform);
			}
		}

		void WriteTransform(
			DirectionalLight& light,
			const Transform& transform,
			GizmoOperation operation)
		{
			if (operation == GizmoOperation::Rotate)
			{
				light.directionToLight =
					-EmissionDirection(transform);
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
						? EmissionDirection(after)
						: light.emissionDirection
				}
			};
		}

		[[nodiscard]] SceneEdit MakeEdit(
			const DirectionalLight& light,
			const Transform&,
			const Transform& after,
			GizmoOperation)
		{
			return DirectionalLightDirectionEdit{
				.lightId = light.id,
				.before = light.directionToLight,
				.after = -EmissionDirection(after)
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

		if constexpr (
			std::is_same_v<Target, DirectionalLight>)
		{
			before.position = context.directionalAnchor;
		}

		Transform after = before;
		glm::mat4 matrix = before.ModelMatrix();

		m_Interaction = DrawGizmo(matrix, context);

		if (!m_Interaction.changed ||
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
		const SceneTransformEditorContext& context)
	{
		return m_Gizmo.DrawInMainViewport(
			ViewportTransformGizmoContext{
				.view = context.view,
				.projection = context.projection,
				.transform = matrix,
				.operation = context.operation,
				.space = context.space,
				.enabled =
					m_WasActive ||
					!context.navigationActive
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
						std::is_same_v<
						Edit,
						DirectionalLightDirectionEdit>)
					{
						if (context.showLights &&
							context.operation ==
							GizmoOperation::Rotate)
						{
							DirectionalLight& light =
								context.scene
								.MainDirectionalLight();

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
			if (context.operation == GizmoOperation::Rotate)
			{
				DirectionalLight& light =
					context.scene.MainDirectionalLight();

				if (light.id == *lightId)
				{
					EditTarget(light, context);
				}
			}

			if (context.operation == GizmoOperation::Translate)
			{
				for (PointLight& light :
					context.scene.PointLights())
				{
					if (light.id == *lightId)
					{
						EditTarget(light, context);
						break;
					}
				}
			}

			if (context.operation == GizmoOperation::Translate ||
				context.operation == GizmoOperation::Rotate)
			{
				for (SpotLight& light :
					context.scene.SpotLights())
				{
					if (light.id == *lightId)
					{
						EditTarget(light, context);
						break;
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