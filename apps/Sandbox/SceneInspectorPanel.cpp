#include "SceneInspectorPanel.hpp"
#include "SandboxScene.hpp"
#include "SceneLightEditing.hpp"
#include "SceneSelection.hpp"

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
            ImGui::TextDisabled(
                "Select an object or light to inspect.");
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
            ImGui::TextDisabled("Selected object no longer exists.");
            return;
        }

        ImGui::TextUnformatted(object->name.c_str());

        ImGui::Text(
            "Object ID: %llu",
            static_cast<unsigned long long>(object->id));

        ImGui::TextWrapped(
            "Model: %s",
            object->modelPath.c_str());

        ImGui::SeparatorText("Transform");

        ImGui::DragFloat3(
            "Position",
            &object->transform.position.x,
            CalculatePositionDragSpeed(context.modelBounds),
            0.0f,
            0.0f,
            "%.6f",
            ImGuiSliderFlags_NoRoundToFormat);

        glm::vec3 rotationDegrees =
            glm::degrees(object->transform.rotationRadians);

        if (ImGui::DragFloat3(
            "Rotation",
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
			ImGui::SeparatorText("Directional Light");

			DirectionalLight& target = context.scene.MainDirectionalLight();
			DirectionalLight candidate = target;

			ImGui::Text(
				"Light ID: %llu",
				static_cast<unsigned long long>(target.id));

			bool changed = false;

			changed |= ImGui::Checkbox(
				"Light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Light editor position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat3(
				"Direction to light",
				&candidate.directionToLight.x,
				0.01f,
				0.0f,
				0.0f,
				"%.3f");

			ImGui::TextDisabled(
				"Position moves the editor marker, not the illumination.");

			ImGui::TextDisabled(
				"Direction points from the surface toward the light.");

			if (changed)
			{
				m_LightValidationError =
					TryApplyLightEdit(target, candidate);
			}
		}

		for (PointLight& target : context.scene.PointLights())
		{
			if (!context.selection.IsLightSelected(target.id))
			{
				continue;
			}

			ImGui::SeparatorText("Selected Point Light");

			PointLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Point light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Point light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Point light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Point light position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat(
				"Point light range",
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

			ImGui::SeparatorText("Selected Spot Light");

			SpotLight candidate = target;
			bool changed = false;

			changed |= ImGui::Checkbox(
				"Spot light enabled",
				&candidate.enabled);

			changed |= ImGui::ColorEdit3(
				"Spot light color",
				&candidate.color.x,
				ImGuiColorEditFlags_Float);

			changed |= ImGui::DragFloat(
				"Spot light intensity",
				&candidate.intensity,
				0.05f,
				0.0f,
				100.0f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);

			changed |= ImGui::DragFloat3(
				"Spot light position",
				&candidate.position.x,
				lightPositionDragSpeed,
				0.0f,
				0.0f,
				"%.6f",
				ImGuiSliderFlags_NoRoundToFormat);

			changed |= ImGui::DragFloat3(
				"Emission direction",
				&candidate.emissionDirection.x,
				0.01f,
				0.0f,
				0.0f,
				"%.3f");

			changed |= ImGui::DragFloat(
				"Spot light range",
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

			if (ImGui::DragFloat(
				"Inner cone half-angle",
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

			if (ImGui::DragFloat(
				"Outer cone half-angle",
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

			ImGui::TextDisabled(
				"Angles are measured from the emission axis.");

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