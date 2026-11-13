#include "SceneHierarchyPanel.hpp"
#include "SceneKeyboardShortcuts.hpp"
#include "SceneEditHistory.hpp"
#include "EditorSectionHeader.hpp"

#include <algorithm>
#include <string>

namespace ForgeSim::Sandbox
{
    namespace
    {
        std::string LightLabel(const char* typeName, SceneLightId id, bool enabled)
        {
            return std::string(typeName) + " " + std::to_string(id) +
                (enabled ? "" : " (disabled)");
        }
        void DrawAssetNode(const Assets::SceneAsset& asset, std::size_t index,
            std::vector<bool>& ancestors, std::size_t depth)
        {
            if (index >= asset.nodes.size()) { ImGui::TextDisabled("Invalid node reference"); return; }
            if (ancestors[index] || depth >= 128) { ImGui::TextDisabled("Hierarchy depth/cycle limit"); return; }
            const auto& node = asset.nodes[index];
            ImGui::PushID(static_cast<int>(index));
            const auto name = node.name.empty() ? "Node " + std::to_string(index) : node.name;
            auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (node.childIndices.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
            const auto label = name + (node.meshIndex ? " [Mesh " + std::to_string(*node.meshIndex) + "]" : " [Node]");
            const bool expanded = ImGui::TreeNodeEx("##AssetNode", flags, "%s", label.c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted("Read-only asset node; editing acts on the model instance.");
                ImGui::Text("Node index: %zu   Children: %zu", index, node.childIndices.size());
                if (node.localTransform.matrix) ImGui::TextUnformatted("Local transform: authored matrix");
                else ImGui::Text("Local translation: %.4f, %.4f, %.4f", node.localTransform.translation[0],
                    node.localTransform.translation[1], node.localTransform.translation[2]);
                if (node.meshIndex && *node.meshIndex < asset.meshes.size())
                {
                    const auto& mesh = asset.meshes[*node.meshIndex];
                    ImGui::Text("Mesh: %s (%zu primitives)", mesh.name.c_str(), mesh.primitives.size());
                    for (std::size_t primitive = 0; primitive < mesh.primitives.size(); ++primitive)
                    {
                        const auto& item = mesh.primitives[primitive];
                        if (item.materialIndex) ImGui::Text("Primitive %zu: Material %zu", primitive, *item.materialIndex + 1);
                        else ImGui::Text("Primitive %zu: Default material", primitive);
                    }
                }
                ImGui::EndTooltip();
            }
            if (expanded)
            {
                ancestors[index] = true;
                for (const auto child : node.childIndices) DrawAssetNode(asset, child, ancestors, depth + 1);
                ancestors[index] = false;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        struct HierarchyRowAction { bool select = false; bool toggle = false; };

        HierarchyRowAction DrawVisibilityRow(const std::string& label, const std::string& buttonId,
            bool selected, bool visible)
        {
            const auto& style = ImGui::GetStyle();
            const float buttonSize = ImGui::GetFrameHeight();
            const float rowWidth = std::max(1.0f,
                ImGui::GetContentRegionAvail().x - buttonSize - style.ItemSpacing.x);
            if (!visible) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0,.5f));
            const bool select = ImGui::Selectable(label.c_str(), selected,
                ImGuiSelectableFlags_None, ImVec2(rowWidth,buttonSize));
            ImGui::PopStyleVar();
            if (!visible) ImGui::PopStyleColor();
            ImGui::SameLine();
            const bool toggle = ImGui::Button(buttonId.c_str(), ImVec2(buttonSize,buttonSize));
            ImGui::SetItemTooltip(visible ? "Hide in Scene (saved with the scene)" : "Show in Scene (saved with the scene)");
            const auto min = ImGui::GetItemRectMin();
            const ImVec2 center{min.x + buttonSize*.5f,min.y + buttonSize*.5f};
            const float radius = buttonSize*.29f;
            const float weight = std::max(1.0f,buttonSize*.055f);
            const auto color = ImGui::GetColorU32(visible ? ImGuiCol_Text : ImGuiCol_TextDisabled);
            auto* draw = ImGui::GetWindowDrawList();
            draw->AddBezierCubic({center.x-radius,center.y},
                {center.x-radius*.5f,center.y-radius}, {center.x+radius*.5f,center.y-radius},
                {center.x+radius,center.y},color,weight);
            draw->AddBezierCubic({center.x-radius,center.y},
                {center.x-radius*.5f,center.y+radius}, {center.x+radius*.5f,center.y+radius},
                {center.x+radius,center.y},color,weight);
            draw->AddCircleFilled(center,radius*.28f,color);
            if (!visible) draw->AddLine({center.x-radius,center.y-radius},
                {center.x+radius,center.y+radius},color,weight);
            return {select,toggle};
        }

    }

    void SceneHierarchyPanel::DrawLightEntry(const char* typeName, SceneLightId id,
        bool enabled, SceneSelection& selection, SandboxScene& scene)
    {
        const auto visible = LightLabel(typeName, id, enabled);
        if (!m_Filter.PassFilter(visible.c_str())) return;
        const auto label = visible + "###Light_" + std::to_string(id);
        const bool visibleInScene = scene.IsLightVisible(id);
        const auto action = DrawVisibilityRow(label, "##VisibilityLight_" + std::to_string(id),
            selection.IsLightSelected(id), visibleInScene);
        if (action.select) selection.SelectLight(id);
        if (action.toggle) scene.SetLightVisible(id, !visibleInScene);
    }

    void SceneHierarchyPanel::Draw(const SceneHierarchyPanelContext& context)
    {
        m_FocusSelectionRequested = false;
        m_DeleteSelectionRequested = false;
        m_DuplicateSelectionRequested = false;
        if (!ImGui::Begin("Hierarchy"))
        {
            ImGui::End();
            return;
        }
        m_FocusSelectionRequested = FocusSelectionShortcutPressed(context.interactionDisabled);
        m_DeleteSelectionRequested = DeleteSelectionShortcutPressed(context.interactionDisabled);
        m_DuplicateSelectionRequested = DuplicateSelectionShortcutPressed(context.interactionDisabled);
        // Compact list density belongs to the Hierarchy, independent of the
        // larger form controls in the Inspector and Debug panels.
        const auto padding = ImGui::GetStyle().FramePadding;
        const auto spacing = ImGui::GetStyle().ItemSpacing;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
            ImVec2(padding.x, std::max(1.0f, padding.y * 0.5f)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
            ImVec2(spacing.x, std::max(1.0f, ImGui::GetFontSize() * 0.125f)));

        ImGui::BeginDisabled(context.interactionDisabled);

        const auto& style = ImGui::GetStyle();
        const float clearWidth = ImGui::CalcTextSize("Clear").x + 2.0f * style.FramePadding.x;
        ImGui::SetNextItemWidth(std::max(1.0f,
            ImGui::GetContentRegionAvail().x - clearWidth - style.ItemSpacing.x));
        bool changed = ImGui::InputTextWithHint("##HierarchySearch", "Search hierarchy...",
            m_Filter.InputBuf, IM_ARRAYSIZE(m_Filter.InputBuf));
        ImGui::SetItemTooltip("Filter names and light types (case-insensitive).\nSeparate alternatives with commas; exclude terms with a leading minus.");
        ImGui::SameLine();
        ImGui::BeginDisabled(m_Filter.InputBuf[0] == '\0');
        if (ImGui::Button("Clear"))
        {
            m_Filter.Clear();
            changed = true;
        }
        ImGui::EndDisabled();
        if (changed)
        {
            m_Filter.Build();
            // Exclusions take precedence regardless of their position in the query.
            std::stable_partition(m_Filter.Filters.begin(), m_Filter.Filters.end(),
                [](const ImGuiTextFilter::ImGuiTextRange& term)
                { return term.b != term.e && term.b[0] == '-'; });
        }
        const bool filtering = m_Filter.IsActive();
        if (context.scene.HasHiddenObjects() && ImGui::Button("Show all objects", ImVec2(-1,0)))
            context.scene.ShowAllObjects();
        if (context.scene.HasHiddenLights() && ImGui::Button("Show all lights", ImVec2(-1,0)))
            context.scene.ShowAllLights();

        // Count matches independently of section expansion. Filtering never
        // changes selection, including a selected item hidden by the query.
        std::size_t objectMatches = 0, lightMatches = 0;
        bool selectedMatches = context.selection.Empty();
        for (const auto& object : context.scene.Objects())
        {
            if (!m_Filter.PassFilter(object.name.c_str())) continue;
            ++objectMatches;
            selectedMatches |= context.selection.IsObjectSelected(object.id);
        }
        const auto countLight = [&](const char* type, const auto& light)
        {
            const auto label = LightLabel(type, light.id, light.enabled);
            if (!m_Filter.PassFilter(label.c_str())) return;
            ++lightMatches;
            selectedMatches |= context.selection.IsLightSelected(light.id);
        };
        const auto& directional = context.scene.MainDirectionalLight();
        if (context.scene.HasDirectionalLight()) countLight("Directional Light", directional);
        for (const auto& light : context.scene.PointLights()) countLight("Point Light", light);
        for (const auto& light : context.scene.SpotLights()) countLight("Spot Light", light);

        if (filtering && !selectedMatches)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
            ImGui::TextWrapped("Selected item is hidden by the filter.");
            ImGui::PopStyleColor();
        }
        if (ImGui::Selectable("None", context.selection.Empty())) context.selection.Clear();

        if (filtering && objectMatches + lightMatches == 0)
            ImGui::TextWrapped("No matching objects or lights.");

        if (!filtering || objectMatches > 0)
        {
            if (changed && filtering) ImGui::SetNextItemOpen(true);
            if (EditorSectionHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();
                if (context.scene.Objects().empty()) ImGui::TextDisabled("No objects.");
                for (const auto& object : context.scene.Objects())
                {
                    if (!m_Filter.PassFilter(object.name.c_str())) continue;
                    const auto id = std::to_string(object.id);
                    const auto label = object.name + "###Object_" + id;
                    const bool visible = context.scene.IsObjectVisible(object.id);
                    const auto* asset = context.inspectModel ? context.inspectModel(object.modelPath) : nullptr;
                    ImGui::PushID(id.c_str());
                    bool expanded = false;
                    if (asset && !asset->rootNodeIndices.empty())
                    {
                        expanded = ImGui::TreeNodeEx("##ModelHierarchy", ImGuiTreeNodeFlags_NoTreePushOnOpen |
                            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);
                        ImGui::SameLine();
                    }
                    const auto action = DrawVisibilityRow(label, "##VisibilityObject_" + id,
                        context.selection.IsObjectSelected(object.id), visible);
                    if (action.select) context.selection.SelectObject(object.id);
                    if (action.toggle) context.scene.SetObjectVisible(object.id, !visible);
                    if (expanded)
                    {
                        // NoTreePushOnOpen leaves indentation to us. Offset asset
                        // arrows beneath the model label, rather than its arrow.
                        const float assetIndent = ImGui::GetTreeNodeToLabelSpacing() * 1.5f;
                        ImGui::Indent(assetIndent);
                        std::vector<bool> ancestors(asset->nodes.size(), false);
                        for (const auto root : asset->rootNodeIndices) DrawAssetNode(*asset, root, ancestors, 0);
                        ImGui::Unindent(assetIndent);
                    }
                    ImGui::PopID();
                }
                ImGui::Unindent();
            }
        }
        if (!filtering || lightMatches > 0)
        {
            if (changed && filtering) ImGui::SetNextItemOpen(true);
            if (EditorSectionHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();
                if (context.scene.HasDirectionalLight())
                    DrawLightEntry("Directional Light", directional.id, directional.enabled, context.selection, context.scene);
                if (lightMatches == 0) ImGui::TextDisabled("No lights.");
                for (const auto& light : context.scene.PointLights())
                    DrawLightEntry("Point Light", light.id, light.enabled, context.selection, context.scene);
                for (const auto& light : context.scene.SpotLights())
                    DrawLightEntry("Spot Light", light.id, light.enabled, context.selection, context.scene);
                ImGui::Unindent();
            }
        }
        ImGui::Separator();
        ImGui::BeginDisabled(!CanDuplicateSceneSelection(context.scene, context.selection));
        if (ImGui::Button("Duplicate selection", ImVec2(-1, 0)))
            m_DuplicateSelectionRequested = true;
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Duplicate at the same position (Ctrl+D), then use Move. One directional light is supported; it cannot be duplicated.");
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!CanDeleteSceneSelection(context.scene, context.selection));
        if (ImGui::Button("Delete selection", ImVec2(-1, 0)))
            m_DeleteSelectionRequested = true;
        ImGui::SetItemTooltip("Remove the selected model instance or light (Delete). Undo restores it.");
        ImGui::EndDisabled();
        ImGui::EndDisabled();
        ImGui::PopStyleVar(2);
        ImGui::End();
    }
}
