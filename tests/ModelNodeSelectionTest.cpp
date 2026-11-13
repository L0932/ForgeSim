#include "ModelNodeQuery.hpp"
#include "SceneSelectionValidation.hpp"
#include "SceneEditHistory.hpp"
#include "ViewportSelectionController.hpp"
#include <iostream>
#include <stdexcept>
#include <glm/ext/matrix_clip_space.hpp>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
    bool Near(glm::vec3 a, glm::vec3 b) { return glm::length(a - b) < 0.0001f; }
    Assets::SceneAsset MakeAsset()
    {
        Assets::SceneAsset asset;
        asset.nodes.resize(5);
        asset.rootNodeIndices = {0};
        asset.nodes[0].name = "Group";
        asset.nodes[0].localTransform.translation = {10,0,0};
        asset.nodes[0].childIndices = {1,2,3};
        asset.nodes[1].name = "Repeated name";
        asset.nodes[1].meshIndex = 0;
        asset.nodes[2].name = "Repeated name";
        asset.nodes[2].meshIndex = 0;
        asset.nodes[2].localTransform.translation = {4,0,-2};
        asset.nodes[3].localTransform.translation = {100,0,0}; // Empty grouping node.
        asset.nodes[4].meshIndex = 0; // Outside the active scene.
        asset.meshes.resize(1);
        Assets::MeshPrimitiveData primitive;
        primitive.geometry.vertices.resize(3);
        primitive.geometry.vertices[0].position = {0,0,0};
        primitive.geometry.vertices[1].position = {1,0,0};
        primitive.geometry.vertices[2].position = {0,1,0};
        primitive.geometry.indices = {0,1,2};
        primitive.materialIndex = 0;
        asset.meshes[0].primitives.push_back(primitive);
        asset.materials.resize(1);
        return asset;
    }
    void TestBoundsAndPicking()
    {
        auto asset = MakeAsset();
        const ModelNodeQuery query{asset};
        Require(query.Find(1) && query.Find(1)->parent == 0 && !query.Find(4), "Only active scene nodes are reachable");
        Require(query.Contains(0, 1) && query.Contains(0, 2) && !query.Contains(1, 2), "Subtree membership distinguishes duplicate-name siblings");
        const auto group = query.WorldBounds(0, glm::mat4{1});
        Require(group && Near(group->minimum, {10,0,-2}) && Near(group->maximum, {15,1,0}), "Grouping bounds enclose descendant geometry, not empty-node origins");
        Require(!query.WorldBounds(3, glm::mat4{1}) && !query.WorldBounds(999, glm::mat4{1}), "Empty/missing nodes have no focus bounds");
        const auto hit = query.Pick(asset, {{10.2f,.2f,5},{0,0,-1}}, glm::mat4{1});
        Require(hit && hit->nodeIndex == 1 && std::abs(hit->distance - 5) < .0001f, "Pick exact triangle under parent transform");
        Require(!query.Pick(asset, {{10.9f,.9f,5},{0,0,-1}}, glm::mat4{1}), "Ray inside AABB but outside triangle must miss");
        Require(!query.Pick(asset, {{1000,0,5},{0,0,-1}}, glm::mat4{1}), "Empty space misses");
        const auto back = query.Pick(asset, {{10.2f,.2f,-5},{0,0,1}}, glm::mat4{1});
        Require(back && back->nodeIndex == 1, "Backface geometry is selectable");
        Transform instance; instance.position = {5,3,0}; instance.scale = {-2,3,1};
        const auto transformed = query.WorldBounds(1, instance.ModelMatrix());
        Require(transformed && Near(transformed->minimum, {-17,3,0}) && Near(transformed->maximum, {-15,6,0}), "Negative nonuniform instance scale in world bounds");
        const auto scaledHit = query.Pick(asset, {{-15.4f,3.6f,5},{0,0,-1}}, instance.ModelMatrix());
        Require(scaledHit && scaledHit->nodeIndex == 1, "Pick under negative instance scale");
        auto local = glm::translate(glm::mat4{1}, {2,3,4});
        std::array<float,16> authored{};
        for (int column=0; column<4; ++column)
            for (int row=0; row<4; ++row) authored[static_cast<std::size_t>(column*4+row)] = local[column][row];
        asset.nodes[1].localTransform.matrix = authored;
        asset.nodes[1].localTransform.translation = {999,999,999};
        const ModelNodeQuery matrixQuery{asset};
        const auto bounds = matrixQuery.WorldBounds(1, glm::mat4{1});
        Require(bounds && Near(bounds->minimum,{12,3,4}) && Near(bounds->maximum,{13,4,4}), "Authored matrices take precedence over TRS");
        asset.nodes[2].localTransform.translation = {0,0,-2};
        asset.nodes[1].localTransform.matrix.reset();
        asset.nodes[1].localTransform.translation = {0,0,0};
        const ModelNodeQuery overlap{asset};
        const auto nearest = overlap.Pick(asset, {{10.2f,.2f,5},{0,0,-1}}, glm::mat4{1});
        Require(nearest && nearest->nodeIndex == 1, "Overlapping nodes pick nearest world-space triangle");
    }
    void TestInvalidAssets()
    {
        for (int kind=0; kind<4; ++kind)
        {
            auto asset = MakeAsset();
            if (kind==0) asset.nodes[1].childIndices = {0};
            if (kind==1) asset.nodes[0].childIndices.push_back(99);
            if (kind==2) asset.nodes[1].meshIndex = 99;
            if (kind==3) asset.rootNodeIndices.push_back(1);
            bool failed=false;
            try { const ModelNodeQuery query{asset}; }
            catch (const std::invalid_argument&) { failed=true; }
            Require(failed, "Reject cycles, invalid indices, and repeated parents safely");
        }
    }
    void TestSelectionAndViewport()
    {
        SandboxScene scene;
        const auto first = scene.Objects().front().id;
        scene.FindObject(first)->modelPath = "room.glb";
        const auto second = scene.AddModelObject("Room copy", "room.glb");
        SceneSelection selection;
        selection.AddObject(first); selection.AddObject(second);
        selection.SelectNode(first, 1, "room.glb");
        Require(!selection.Empty() && selection.SelectedObjectIds().empty() && !selection.SelectedLightId() &&
            selection.IsNodeSelected(first,1) && !selection.IsNodeSelected(second,1), "Node selection is exclusive and instance-specific");
        Require(!CanDeleteSceneSelection(scene, selection) && !CanDuplicateSceneSelection(scene, selection), "Node selection must not delete or duplicate its owner");
        ClearMissingSceneSelection(selection, scene);
        Require(selection.SelectedNode().has_value(), "Existing node owner keeps selection");
        scene.FindObject(first)->modelPath = "replacement.glb";
        ClearMissingSceneSelection(selection, scene);
        Require(selection.Empty(), "Changed model reference clears stale node selection");
        selection.SelectNode(second,1,"room.glb");
        Require(scene.RemoveObject(second), "Remove owner");
        ClearMissingSceneSelection(selection, scene);
        Require(selection.Empty(), "Deleted owner clears node selection");
        selection.SelectNode(first,1,"replacement.glb");
        selection.AddObject(first);
        Require(!selection.SelectedNode() && selection.IsObjectSelected(first), "Ctrl/Shift object selection replaces node selection");
        selection.SelectNode(first,1,"replacement.glb");
        selection.SelectLight(scene.MainDirectionalLight().id);
        Require(!selection.SelectedNode() && selection.SelectedLightId(), "Light selection replaces node selection");
        const auto third = scene.AddModelObject("Nearer room", "replacement.glb");
        Platform::InputState input;
        ViewportSelectionController picking;
        const glm::mat4 view{1};
        const auto projection = glm::perspective(glm::radians(60.0f),1.0f,.1f,100.0f);
        const auto callback = [=](const Renderer::Ray&, const SandboxObject& object) -> std::optional<ModelNodeHit>
        { return ModelNodeHit{object.id == first ? 1u : 2u, object.id == first ? 10.0f : 5.0f}; };
        input.SetMouseButtonState(Platform::MouseButton::Left,true);
        picking.Update(input,{50,50},{100,100},view,projection,{{-1,-1,-1},{1,1,1}},scene.Objects(),{},selection,
            false,false,false,true,callback);
        Require(selection.IsNodeSelected(third,2), "Alt-click chooses nearest node across instances");
        picking.Update(input,{50,50},{100,100},view,projection,{{-1,-1,-1},{1,1,1}},scene.Objects(),{},selection,
            true,false,false,true,callback);
        Require(selection.IsNodeSelected(third,2), "Suppressed picking preserves node selection");
        const auto miss=[](const Renderer::Ray&, const SandboxObject&) -> std::optional<ModelNodeHit> { return {}; };
        picking.Update(input,{50,50},{100,100},view,projection,{{-1,-1,-1},{1,1,1}},scene.Objects(),{},selection,
            false,false,false,true,miss);
        Require(selection.Empty(), "Alt-click empty space clears node selection");
    }
}
int main()
{
    try { TestBoundsAndPicking(); TestInvalidAssets(); TestSelectionAndViewport(); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    std::cout << "Model node selection, subtree bounds, picking and selection safety passed\n";
}
