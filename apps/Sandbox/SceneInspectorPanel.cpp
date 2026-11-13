#include "SceneInspectorPanel.hpp"
#include "EditorSectionHeader.hpp"
#include "InspectorPropertyWidgets.hpp"
#include "SandboxScene.hpp"
#include "SceneLightEditing.hpp"
#include "SceneSelection.hpp"
#include "SceneTransformEditor.hpp"
#include "ObjectTransformSelection.hpp"

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
        m_ObjectEdit.reset();
        m_EditTargets.clear();
    }

    void SceneInspectorPanel::FinishObjectEdit(SceneTransformEditor* editor)
    {
        if (m_ObjectEdit && editor) editor->RecordObjectEdits(std::move(*m_ObjectEdit));
        m_ObjectEdit.reset();
        m_EditTargets.clear();
    }

    void SceneInspectorPanel::Draw(
        const SceneInspectorPanelContext& context)
    {
        const auto ids = context.selection.SelectedObjectIds();
        if (m_ObjectEdit && (!std::equal(ids.begin(), ids.end(), m_EditTargets.begin(), m_EditTargets.end()) ||
            !ImGui::IsAnyItemActive())) FinishObjectEdit(context.transformEditor);
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
        else if (context.selection.SelectedNode())
        {
            DrawNode(context);
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

    void SceneInspectorPanel::DrawNode(const SceneInspectorPanelContext& context)
    {
        const auto& selected = *context.selection.SelectedNode();
        const auto* asset = context.materials.asset;
        const auto* owner = context.scene.FindObject(selected.objectId);
        const auto* record = context.nodeQuery ? context.nodeQuery->Find(selected.nodeIndex) : nullptr;
        if (!asset || !owner || !record || selected.nodeIndex >= asset->nodes.size())
        { ImGui::TextWrapped("Selected node is unavailable."); return; }
        const auto& node = asset->nodes[selected.nodeIndex];
        const auto name = node.name.empty() ? "Node " + std::to_string(selected.nodeIndex) : node.name;
        ImGui::TextWrapped("%s", name.c_str());
        ImGui::TextWrapped("Model instance: %s", owner->name.c_str());
        ImGui::Text("Node index: %zu   Children: %zu", selected.nodeIndex, node.childIndices.size());
        if (record->parent)
        {
            const auto& parent = asset->nodes[*record->parent];
            ImGui::TextWrapped("Parent: %s (node %zu)", parent.name.empty() ? "Unnamed" : parent.name.c_str(), *record->parent);
        }
        else ImGui::TextUnformatted("Parent: model instance");
        ImGui::TextWrapped("Read-only node inspection. Use the model row for instance transforms, deletion, and duplication. Node editing and persistence follow separately.");
        if (EditorSectionHeader("Local Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& t = node.localTransform;
            if (t.matrix)
            {
                ImGui::TextUnformatted("Authored matrix (rows):");
                for (std::size_t row = 0; row < 4; ++row)
                    ImGui::Text("%.4f  %.4f  %.4f  %.4f", (*t.matrix)[row], (*t.matrix)[4 + row],
                        (*t.matrix)[8 + row], (*t.matrix)[12 + row]);
            }
            else
            {
                ImGui::TextWrapped("Translation: %.4f, %.4f, %.4f", t.translation[0], t.translation[1], t.translation[2]);
                ImGui::TextWrapped("Rotation quaternion XYZW: %.4f, %.4f, %.4f, %.4f",
                    t.rotation[0], t.rotation[1], t.rotation[2], t.rotation[3]);
                ImGui::TextWrapped("Scale: %.4f, %.4f, %.4f", t.scale[0], t.scale[1], t.scale[2]);
            }
        }
        if (EditorSectionHeader("World Bounds", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto bounds = context.nodeQuery->WorldBounds(selected.nodeIndex, owner->transform.ModelMatrix());
            if (!bounds) ImGui::TextWrapped("No geometry in this node or its descendants; nothing to frame.");
            else
            {
                ImGui::TextWrapped("Min: %.4f, %.4f, %.4f", bounds->minimum.x, bounds->minimum.y, bounds->minimum.z);
                ImGui::TextWrapped("Max: %.4f, %.4f, %.4f", bounds->maximum.x, bounds->maximum.y, bounds->maximum.z);
            }
        }
        auto materials = context.materials;
        materials.materialIndices.emplace();
        bool hasDefault = false;
        for (std::size_t index = 0; index < asset->nodes.size(); ++index)
        {
            if (!context.nodeQuery->Contains(selected.nodeIndex, index)) continue;
            const auto meshIndex = asset->nodes[index].meshIndex;
            if (!meshIndex || *meshIndex >= asset->meshes.size()) continue;
            const auto& mesh = asset->meshes[*meshIndex];
            ImGui::TextWrapped("Node %zu: Mesh %zu — %s (%zu primitives)", index, *meshIndex,
                mesh.name.empty() ? "Unnamed" : mesh.name.c_str(), mesh.primitives.size());
            for (const auto& primitive : mesh.primitives)
            {
                if (!primitive.materialIndex) { hasDefault = true; continue; }
                auto& indices = *materials.materialIndices;
                if (std::find(indices.begin(), indices.end(), *primitive.materialIndex) == indices.end())
                    indices.push_back(*primitive.materialIndex);
            }
        }
        if (hasDefault) ImGui::TextWrapped("Some primitives use the default glTF material.");
        DrawMaterials(materials);
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

        const auto objectBounds = object->localBounds.value_or(context.modelBounds);
        const auto count = context.selection.SelectedObjectIds().size();
        if (count > 1)
        {
            ImGui::Text("%zu objects selected", count);
            ImGui::TextWrapped("Active: %s", object->name.c_str());
            ImGui::TextWrapped("Transforms apply a shared delta to all selected objects. Rotation and scale use individual pivots. Values and materials below describe the active object.");
        }
        else ImGui::TextWrapped("%s", object->name.c_str());
        ImGui::Text("Object ID: %llu", static_cast<unsigned long long>(object->id));
        ImGui::TextWrapped("Model: %s", object->modelPath.c_str());

        if (context.transformEditor)
        {
            if (EditorSectionHeader("Pivot", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("Pivot: %.4f, %.4f, %.4f",
                    object->transform.pivot.x, object->transform.pivot.y, object->transform.pivot.z);
                const bool center = ImGui::Button("Use bounds center", ImVec2(-1.0f, 0.0f));
                const bool origin = ImGui::Button("Use authored origin", ImVec2(-1.0f, 0.0f));
                if (center || origin)
                {
                    FinishObjectEdit(context.transformEditor);
                    static_cast<void>(context.transformEditor->SetSelectedObjectPivots(
                        context.scene, context.selection, context.modelBounds, center));
                    return; // The atomic scene update invalidates borrowed object pointers.
                }
                ImGui::TextWrapped("Changing pivots preserves placement. Position locates each pivot.");
            }
        }
        if (EditorSectionHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto apply = [&](const Transform& before, const Transform& after, GizmoOperation operation)
            {
                auto batch = BuildObjectTransformEdits(context.scene, context.selection, before, after, operation);
                if (!batch) return;
                if (!m_ObjectEdit)
                {
                    m_ObjectEdit = *batch;
                    const auto targets = context.selection.SelectedObjectIds();
                    m_EditTargets.assign(targets.begin(), targets.end());
                }
                else
                    for (std::size_t i = 0; i < batch->edits.size(); ++i)
                        std::get<ObjectTransformEdit>(m_ObjectEdit->edits[i]).after =
                            std::get<ObjectTransformEdit>(batch->edits[i]).after;
                for (const auto& edit : batch->edits)
                {
                    const auto& item = std::get<ObjectTransformEdit>(edit);
                    context.scene.FindObject(item.objectId)->transform = item.after;
                }
            };
            auto before = object->transform;
            auto after = before;
            if (InspectorUI::DragFloat3("##Position", "Position", &after.position.x,
                CalculatePositionDragSpeed(objectBounds), 0.0f, 0.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat))
                apply(before, after, GizmoOperation::Translate);
            before = object->transform;
            after = before;
            auto degrees = glm::degrees(before.rotationRadians);
            if (InspectorUI::DragFloat3("##Rotation", "Rotation", &degrees.x, 1.0f, 0.0f, 0.0f, "%.1f deg"))
            {
                after.rotationRadians = glm::radians(degrees);
                apply(before, after, GizmoOperation::Rotate);
            }
            before = object->transform;
            after = before;
            if (InspectorUI::DragFloat3("##Scale", "Scale", &after.scale.x, 0.01f))
                apply(before, after, GizmoOperation::Scale);
        }
        if (!ImGui::IsAnyItemActive()) FinishObjectEdit(context.transformEditor);
        DrawMaterials(context.materials);
    }

    void SceneInspectorPanel::DrawMaterials(const ModelMaterialInspection& inspection)
    {
        if (!EditorSectionHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) return;
        ImGui::TextDisabled("Read-only asset materials");
        if (!inspection.asset)
        {
            ImGui::TextWrapped("Material resources are unavailable.");
            return;
        }
        const auto& asset = *inspection.asset;
        if (asset.materials.empty())
        {
            ImGui::TextWrapped("No authored materials. Uses the default glTF material (white, metallic 1, roughness 1).");
            return;
        }
        const auto count = inspection.materialIndices ? inspection.materialIndices->size() : asset.materials.size();
        ImGui::Text("%zu material(s)", count);
        if (count == 0) ImGui::TextWrapped("No authored materials associated with the selected subtree.");
        for (std::size_t index = 0; index < asset.materials.size(); ++index)
        {
            if (inspection.materialIndices && std::find(inspection.materialIndices->begin(),
                inspection.materialIndices->end(), index) == inspection.materialIndices->end()) continue;
            ImGui::PushID(static_cast<int>(index));
            const auto label = "Material " + std::to_string(index + 1);
            if (!ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            { ImGui::PopID(); continue; }
            const auto& material = asset.materials[index];
            const auto& pbr = material.metallicRoughness;
            ImGui::Text("Base RGBA: %.3f, %.3f, %.3f, %.3f", pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
            ImGui::Text("Metallic: %.3f   Roughness: %.3f", pbr.metallicFactor, pbr.roughnessFactor);
            ImGui::Text("Emissive RGB: %.3f, %.3f, %.3f", material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);
            ImGui::Text("Alpha: %s   Double-sided: %s", material.alphaMode == Assets::AlphaMode::Opaque ? "Opaque" :
                material.alphaMode == Assets::AlphaMode::Mask ? "Mask" : "Blend", material.doubleSided ? "Yes" : "No");
            if (material.alphaMode == Assets::AlphaMode::Mask) ImGui::Text("Alpha cutoff: %.3f", material.alphaCutoff);
            if (material.normalTexture) ImGui::Text("Normal scale: %.3f", material.normalTexture->scale);
            if (material.occlusionTexture) ImGui::Text("Occlusion strength: %.3f", material.occlusionTexture->strength);
            const int assignedMapCount = static_cast<int>(pbr.baseColorTexture.has_value()) +
                static_cast<int>(pbr.metallicRoughnessTexture.has_value()) + static_cast<int>(material.normalTexture.has_value()) +
                static_cast<int>(material.occlusionTexture.has_value()) + static_cast<int>(material.emissiveTexture.has_value());
            const int columns = std::min(std::max(assignedMapCount, 1),
                std::clamp(static_cast<int>(ImGui::GetContentRegionAvail().x / 260.0f), 1, 3));
            if (assignedMapCount > 0 && ImGui::BeginTable("TextureMaps", columns, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX))
            {
                const auto drawMap = [&](const char* name, const Assets::MaterialTextureReference* reference,
                    std::size_t slot, const char* usage)
                {
                    if (!reference) return;
                    ImGui::TableNextColumn();
                    ImGui::PushID(name);
                    ImGui::Separator();
                    ImGui::TextUnformatted(name);
                    ImGui::TextWrapped("%s", usage);
                    ImGui::Text("Texture %zu   UV set %u", reference->textureIndex, reference->textureCoordinateSet);
                    const auto& resources = asset.textureResources;
                    if (reference->textureIndex < resources.textures.size())
                    {
                        const auto imageIndex = resources.textures[reference->textureIndex].imageIndex;
                        if (imageIndex < resources.images.size())
                        {
                            const auto& image = resources.images[imageIndex];
                            ImGui::Text("Image %zu: %u x %u (%u channels)", imageIndex, image.width, image.height, image.channelCount);
                            const auto handle = index < inspection.textureHandles.size() ? inspection.textureHandles[index][slot] : 0;
                            if (handle && image.width && image.height)
                            {
                                const float width = std::min(128.0f, std::max(1.0f, ImGui::GetContentRegionAvail().x));
                                const float height = std::min(128.0f, width * static_cast<float>(image.height) / static_cast<float>(image.width));
                                const float fittedWidth = height * static_cast<float>(image.width) / static_cast<float>(image.height);
                                ImGui::Image(ImTextureRef{static_cast<ImTextureID>(handle)}, ImVec2(fittedWidth, height));
                                if (ImGui::IsItemHovered())
                                {
                                    ImGui::BeginTooltip();
                                    const float largeWidth = std::min(320.0f, 320.0f * static_cast<float>(image.width) / static_cast<float>(image.height));
                                    ImGui::Image(ImTextureRef{static_cast<ImTextureID>(handle)},
                                        ImVec2(largeWidth, largeWidth * static_cast<float>(image.height) / static_cast<float>(image.width)));
                                    ImGui::EndTooltip();
                                }
                            }
                            else ImGui::TextDisabled("Preview unavailable");
                        }
                        else ImGui::TextDisabled("Image unavailable");
                    }
                    else ImGui::TextDisabled("Texture unavailable");
                    ImGui::PopID();
                };
                drawMap("Base Color", pbr.baseColorTexture ? &*pbr.baseColorTexture : nullptr, 0, "sRGB color; multiplied by base RGBA");
                drawMap("Metallic / Roughness", pbr.metallicRoughnessTexture ? &*pbr.metallicRoughnessTexture : nullptr, 1, "Linear data: G = roughness, B = metallic; multiplied by factors");
                drawMap("Normal", material.normalTexture ? &material.normalTexture->texture : nullptr, 2, "Linear tangent-space normal map");
                drawMap("Occlusion", material.occlusionTexture ? &material.occlusionTexture->texture : nullptr, 3, "Linear data: R = ambient occlusion");
                drawMap("Emissive", material.emissiveTexture ? &*material.emissiveTexture : nullptr, 4, "sRGB color; multiplied by emissive RGB");
                ImGui::EndTable();
            }
            std::string missingMaps;
            const auto addMissing = [&](const char* name, bool assigned)
            {
                if (assigned) return;
                if (!missingMaps.empty()) missingMaps += ", ";
                missingMaps += name;
            };
            addMissing("Base Color", pbr.baseColorTexture.has_value());
            addMissing("Metallic / Roughness", pbr.metallicRoughnessTexture.has_value());
            addMissing("Normal", material.normalTexture.has_value());
            addMissing("Occlusion", material.occlusionTexture.has_value());
            addMissing("Emissive", material.emissiveTexture.has_value());
            if (!missingMaps.empty())
            {
                ImGui::Separator();
                ImGui::TextWrapped("Unassigned maps: %s", missingMaps.c_str());
                ImGui::TextDisabled("Uses material defaults for these slots.");
            }
            ImGui::TreePop();
            ImGui::PopID();
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