#include "SceneInspectorPanel.hpp"
#include "EditorSectionHeader.hpp"
#include "InspectorPropertyWidgets.hpp"
#include "SandboxScene.hpp"
#include "SceneLightEditing.hpp"
#include "SceneSelection.hpp"
#include "SceneTransformEditor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <imgui.h>
#include <glm/trigonometric.hpp>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
    float SceneInspectorPanel::CalculatePositionDragSpeed(
        const Renderer::AxisAlignedBoundingBox& bounds)
    {
        const glm::vec3 extent =
            bounds.maximum - bounds.minimum;

        if (!std::isfinite(extent.x) ||
            !std::isfinite(extent.y) ||
            !std::isfinite(extent.z) ||
            extent.x < 0.0f ||
            extent.y < 0.0f ||
            extent.z < 0.0f)
        {
            return 0.001f;
        }

        const float size =
            std::max({ extent.x, extent.y, extent.z });

        return size > 0.0f
            ? std::clamp(size * 0.001f, 0.000001f, 10.0f)
            : 0.001f;
    }

    void SceneInspectorPanel::Reset() noexcept
    {
        m_LightValidationError.reset();
    }

    void SceneInspectorPanel::Draw(
        const SceneInspectorPanelContext& context)
    {
        if (!ImGui::Begin("Inspector"))
        {
            ImGui::End();
            return;
        }

        if (m_LightValidationError &&
            !context.selection.IsLightSelected(
                m_LightValidationError->lightId))
        {
            Reset();
        }

        ImGui::BeginDisabled(context.editingDisabled);

        if (context.selection.Empty())
        {
            ImGui::PushTextWrapPos();
            ImGui::TextDisabled(
                "Select an object or light to inspect.");
            ImGui::PopTextWrapPos();
        }
        else if (context.selection.SelectedObjectId())
        {
            DrawObject(context);
        }
        else
        {
            DrawLights(context);
        }

        ImGui::EndDisabled();
        ImGui::End();
    }

    void SceneInspectorPanel::DrawObject(
        const SceneInspectorPanelContext& context)
    {
        const auto objectId =
            context.selection.SelectedObjectId();

        SandboxObject* object =
            objectId
            ? context.scene.FindObject(*objectId)
            : nullptr;

        if (!object)
        {
            ImGui::PushTextWrapPos();
            ImGui::TextDisabled("Selected object no longer exists.");
            ImGui::PopTextWrapPos();
            return;
        }

        const auto& objectBounds = object->localBounds.value_or(context.modelBounds);
        ImGui::TextWrapped("%s", object->name.c_str());

        ImGui::Text(
            "Object ID: %llu",
            static_cast<unsigned long long>(object->id));

        ImGui::TextWrapped(
            "Model: %s",
            object->modelPath.c_str());

        if (context.transformEditor)
        {
            if (EditorSectionHeader("Pivot", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("Pivot: %.4f, %.4f, %.4f",
                    object->transform.pivot.x, object->transform.pivot.y, object->transform.pivot.z);
                if (ImGui::Button("Use bounds center", ImVec2(-1.0f, 0.0f)))
                {
                    // Half each endpoint separately to avoid overflowing their sum.
                    const glm::vec3 center = objectBounds.minimum * 0.5f +
                        objectBounds.maximum * 0.5f;
                    static_cast<void>(context.transformEditor->SetObjectPivot(*object, center));
                }
                if (ImGui::Button("Use authored origin", ImVec2(-1.0f, 0.0f)))
                    static_cast<void>(context.transformEditor->SetObjectPivot(*object, glm::vec3{0.0f}));
                ImGui::TextWrapped("Changing the pivot preserves placement. Position locates the pivot.");
            }
        }

        if (EditorSectionHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {

            InspectorUI::DragFloat3(
                "##Position", "Position",
                &object->transform.position.x,
                CalculatePositionDragSpeed(objectBounds),
                0.0f,
                0.0f,
                "%.6f",
                ImGuiSliderFlags_NoRoundToFormat);

            glm::vec3 rotationDegrees =
                glm::degrees(object->transform.rotationRadians);

            if (InspectorUI::DragFloat3(
                "##Rotation", "Rotation",
                &rotationDegrees.x,
                1.0f,
                0.0f,
                0.0f,
                "%.1f deg"))
            {
                object->transform.rotationRadians =
                    glm::radians(rotationDegrees);
            }
        }
    }

    void SceneInspectorPanel::DrawLights(
        const SceneInspectorPanelContext& context)
    {
		if (m_LightValidationError &&
			!context.selection.IsLightSelected(
				m_LightValidationError->lightId))
		{
			m_LightValidationError.reset();
		}

		const float lightPositionDragSpeed =
			CalculatePositionDragSpeed(context.modelBounds);

		if (context.selection.IsLightSelected(
			context.scene.MainDirectionalLight().id))
		{
            if (EditorSectionHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
            {

                DirectionalLight& target = context.scene.MainDirectionalLight();
                DirectionalLight candidate = target;

                ImGui::Text(
                    "Light ID: %llu",
                    static_cast<unsigned long long>(target.id));

                bool changed = false;

                changed |= ImGui::Checkbox(
                    "Light enabled",
                    &candidate.enabled);

                changed |= InspectorUI::ColorEdit3(
                    "##Light color", "Color",
                    &candidate.color.x,
                    ImGuiColorEditFlags_Float);

                changed |= InspectorUI::DragFloat(
                    "##Light intensity", "Intensity",
                    &candidate.intensity,
                    0.05f,
                    0.0f,
                    100.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp);

                changed |= InspectorUI::DragFloat3(
                    "##Light editor position", "Editor position",
                    &candidate.position.x,
                    lightPositionDragSpeed,
                    0.0f,
                    0.0f,
                    "%.6f",
                    ImGuiSliderFlags_NoRoundToFormat);

                changed |= InspectorUI::DragFloat3(
                    "##Direction to light", "Direction to light",
                    &candidate.directionToLight.x,
                    0.01f,
                    0.0f,
                    0.0f,
                    "%.3f");

                ImGui::PushTextWrapPos();
                ImGui::TextDisabled(
                    "Position moves the editor marker, not the illumination.");
                ImGui::PopTextWrapPos();

                ImGui::PushTextWrapPos();
                ImGui::TextDisabled(
                    "Direction points from the surface toward the light.");
                ImGui::PopTextWrapPos();

                if (changed)
                {
                    m_LightValidationError =
                        TryApplyLightEdit(target, candidate);
                }
            }
		}

		for (PointLight& target : context.scene.PointLights())
		{
			if (!context.selection.IsLightSelected(target.id))
			{
				continue;
			}

            if (!EditorSectionHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
                break;

			PointLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Point light enabled",
				&candidate.enabled);

			changed |= InspectorUI::ColorEdit3(
			    "##Point light color", "Color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= InspectorUI::DragFloat(
			    "##Point light intensity", "Intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= InspectorUI::DragFloat3(
			    "##Point light position", "Position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= InspectorUI::DragFloat(
			    "##Point light range", "Range",
				&candidate.range,
				lightPositionDragSpeed,
				0.000001f,
				std::numeric_limits<float>::max(),
				"%.6f",
				ImGuiSliderFlags_AlwaysClamp |
				ImGuiSliderFlags_NoRoundToFormat);

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}

			break;
		}

		for (SpotLight& target : context.scene.SpotLights())
		{
			if (!context.selection.IsLightSelected(target.id))
			{
				continue;
			}

            if (!EditorSectionHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen))
                break;

			SpotLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Spot light enabled",
				&candidate.enabled);

			changed |= InspectorUI::ColorEdit3(
			    "##Spot light color", "Color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= InspectorUI::DragFloat(
			    "##Spot light intensity", "Intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= InspectorUI::DragFloat3(
			    "##Spot light position", "Position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= InspectorUI::DragFloat3(
			    "##Emission direction", "Emission direction",
				&candidate.emissionDirection.x,
				0.01f,
				0.0f,
				0.0f,
				"%.3f");

			changed |= InspectorUI::DragFloat(
			    "##Spot light range", "Range",
				&candidate.range,
				lightPositionDragSpeed,
				0.000001f,
				std::numeric_limits<float>::max(),
				"%.6f",
				ImGuiSliderFlags_AlwaysClamp |
				ImGuiSliderFlags_NoRoundToFormat);

			float innerConeDegrees =
				glm::degrees(candidate.innerConeAngleRadians);

			float outerConeDegrees =
				glm::degrees(candidate.outerConeAngleRadians);

			if (InspectorUI::DragFloat(
			    "##Inner cone half-angle", "Inner half-angle",
				&innerConeDegrees,
				0.1f,
				0.0f,
				0.0f,
				"%.2f deg"))
			{
				candidate.innerConeAngleRadians =
					glm::radians(innerConeDegrees);

				changed = true;
			}

			if (InspectorUI::DragFloat(
			    "##Outer cone half-angle", "Outer half-angle",
				&outerConeDegrees,
				0.1f,
				0.0f,
				0.0f,
				"%.2f deg"))
			{
				candidate.outerConeAngleRadians =
					glm::radians(outerConeDegrees);

				changed = true;
			}

			ImGui::PushTextWrapPos();
			ImGui::TextDisabled(
				"Angles are measured from the emission axis.");
			ImGui::PopTextWrapPos();

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}

			break;
		}

		if (m_LightValidationError)
		{
			const LightValidationError& error =
				*m_LightValidationError;

			ImGui::TextWrapped(
				"Light %llu - %.*s: %.*s",
				static_cast<unsigned long long>(error.lightId),
				static_cast<int>(error.property.size()),
				error.property.data(),
				static_cast<int>(error.message.size()),
				error.message.data());
		}
    }
}