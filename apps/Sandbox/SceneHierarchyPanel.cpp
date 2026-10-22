#include "SceneHierarchyPanel.hpp"

#include <string>

#include <imgui.h>

namespace ForgeSim::Sandbox
{
    void SceneHierarchyPanel::DrawLightEntry(
        const char* typeName,
        SceneLightId id,
        bool enabled,
        SceneSelection& selection)
    {
        const std::string idText = std::to_string(id);

        const std::string label =
            std::string{ typeName } + " " + idText +
            (enabled ? "" : " (disabled)") +
            "###Light_" + idText;

        if (ImGui::Selectable(
            label.c_str(),
            selection.IsLightSelected(id)))
        {
            selection.SelectLight(id);
        }
    }

    void SceneHierarchyPanel::Draw(
        const SceneHierarchyPanelContext& context) const
    {
        if (!ImGui::Begin("Hierarchy"))
        {
            ImGui::End();
            return;
        }

        ImGui::BeginDisabled(context.interactionDisabled);

        if (ImGui::Selectable(
            "None",
            context.selection.Empty()))
        {
            context.selection.Clear();
        }

        if (ImGui::CollapsingHeader(
            "Objects",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (context.scene.Objects().empty())
            {
                ImGui::TextDisabled("No objects.");
            }

            for (const SandboxObject& object :
                context.scene.Objects())
            {
                const std::string label =
                    object.name +
                    "###Object_" +
                    std::to_string(object.id);

                if (ImGui::Selectable(
                    label.c_str(),
                    context.selection.IsObjectSelected(object.id)))
                {
                    context.selection.SelectObject(object.id);
                }
            }
        }

        if (ImGui::CollapsingHeader(
            "Lights",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            const DirectionalLight& directional =
                context.scene.MainDirectionalLight();

            DrawLightEntry(
                "Directional Light",
                directional.id,
                directional.enabled,
                context.selection);

            for (const PointLight& light :
                context.scene.PointLights())
            {
                DrawLightEntry(
                    "Point Light",
                    light.id,
                    light.enabled,
                    context.selection);
            }

            for (const SpotLight& light :
                context.scene.SpotLights())
            {
                DrawLightEntry(
                    "Spot Light",
                    light.id,
                    light.enabled,
                    context.selection);
            }
        }

        ImGui::EndDisabled();
        ImGui::End();
    }
}