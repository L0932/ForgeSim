#include "ObjectTransformSelection.hpp"
#include "SceneAssetPath.hpp"
#include "SceneTransformEditor.hpp"
#include "SceneSelectionValidation.hpp"
#include "SelectionFocus.hpp"
#include "SceneSerializer.hpp"
#include "ViewportSelectionController.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
// Script only the UI boundary; exercise the real editor drag and history logic.
namespace
{
    glm::vec3 gizmoTranslation{0};
    bool gizmoChanged = false;
    bool gizmoActive = false;
}
namespace ForgeSim::Sandbox
{
    void ViewportTransformGizmo::BeginFrame() {}
    GizmoInteraction ViewportTransformGizmo::Draw(const ViewportTransformGizmoContext& context)
    {
        if (!context.enabled) return {};
        if (gizmoChanged) context.transform[3] += glm::vec4{gizmoTranslation, 0.0f};
        return {gizmoChanged, false, gizmoActive};
    }
}
namespace
{
    void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
    bool Near(glm::vec3 a, glm::vec3 b) { return glm::length(a - b) < 0.0001f; }
    void SelectionGestures()
    {
        SceneSelection selection;
        const std::array<SandboxObjectId, 5> order{1, 2, 3, 4, 5};
        selection.ClickObject(2, false, false, order);
        selection.ClickObject(4, false, true, order);
        Require(selection.SelectedObjectIds().size() == 3 && selection.IsObjectSelected(2) &&
            selection.IsObjectSelected(3) && selection.SelectedObjectId() == 4, "Shift selects range and clicked active member");
        selection.ClickObject(1, false, true, order);
        Require(selection.SelectedObjectIds().size() == 2 && selection.IsObjectSelected(2) &&
            selection.IsObjectSelected(1), "Repeated Shift retains original anchor and replaces range");
        selection.ClickObject(5, true, false, order);
        selection.ClickObject(1, true, false, order);
        Require(selection.SelectedObjectIds().size() == 2 && selection.SelectedObjectId() == 5,
            "Ctrl toggles and falls back to previous active object");
        const std::array<SandboxObjectId, 3> filtered{1, 3, 5};
        selection.ClickObject(1, false, false, filtered);
        selection.ClickObject(5, false, true, filtered);
        Require(selection.SelectedObjectIds().size() == 3 && !selection.IsObjectSelected(2) &&
            !selection.IsObjectSelected(4), "Filtered ranges contain only displayed rows");
        selection.ClickObject(9, true, true, filtered);
        Require(selection.SelectedObjectIds().size() == 4, "Missing range endpoint safely adds clicked object");
        selection.SelectLight(1);
        Require(selection.SelectedObjectIds().empty() && selection.IsLightSelected(1), "Light clears object set");
        selection.ClickObject(2, false, true);
        selection.ClickObject(3, false, true);
        selection.ClickObject(3, false, true);
        Require(selection.SelectedObjectIds().size() == 2 && !selection.SelectedLightId(), "Scene Shift adds without duplicates");
        selection.ClickObject(2, true, false);
        selection.ClickObject(3, true, false);
        Require(selection.Empty(), "Removing last member empties selection");
    }
    struct Fixture
    {
        SandboxScene scene{SceneDocument{}};
        SceneSelection selection;
        std::array<SandboxObjectId, 4> ids{};
        Fixture()
        {
            for (std::size_t i = 0; i < ids.size(); ++i)
            {
                ids[i] = scene.AddModelObject("Cube", std::string{BuiltinCubeReference});
                scene.FindObject(ids[i])->transform.position = {static_cast<float>(i) * 3.0f, 0, 0};
                scene.FindObject(ids[i])->localBounds = Renderer::AxisAlignedBoundingBox{{-.5f,-.5f,-.5f},{.5f,.5f,.5f}};
            }
            selection.AddObject(ids[0]);
            selection.AddObject(ids[2]);
            scene.SetObjectVisible(ids[2], false);
        }
    };
    void BatchHistory()
    {
        Fixture f;
        SceneEditHistory history;
        Require(history.DuplicateSelection(f.scene, f.selection), "Duplicate group");
        const auto copies = f.selection;
        Require(f.scene.Objects().size() == 6 && f.selection.SelectedObjectIds().size() == 2,
            "Every member copied and copies selected");
        const auto a = f.selection.SelectedObjectIds()[0], b = f.selection.SelectedObjectIds()[1];
        Require(a != b && a != f.ids[0] && f.scene.FindObject(a)->name == "Cube (Copy 1)" &&
            f.scene.FindObject(b)->name == "Cube (Copy 2)" && !f.scene.IsObjectVisible(b), "Unique IDs, names, hidden state");
        Require(history.Undo(f.scene, &f.selection) && f.scene.Objects().size() == 4 &&
            f.selection.IsObjectSelected(f.ids[0]) && f.selection.IsObjectSelected(f.ids[2]) && !history.CanUndo(),
            "One undo removes all copies and restores original selection");
        Require(history.Redo(f.scene, &f.selection) && f.selection.IsObjectSelected(a) &&
            f.selection.IsObjectSelected(b), "One redo restores group identities");
        Require(history.DeleteSelection(f.scene, f.selection) && f.selection.Empty() && f.scene.Objects().size() == 4,
            "Delete group in one action");
        Require(history.Undo(f.scene, &f.selection) && f.scene.Objects()[4].id == a && f.scene.Objects()[5].id == b &&
            !f.scene.IsObjectVisible(b), "Undo restores list order, selection and visibility");
        Require(history.Redo(f.scene, &f.selection) && !f.scene.FindObject(a) && !f.scene.FindObject(b), "Redo deletes whole group");
        f.selection = SceneSelection{};
        f.selection.AddObject(f.ids[2]); f.selection.AddObject(f.ids[0]);
        Require(history.DeleteSelection(f.scene, f.selection) && history.Undo(f.scene, &f.selection), "Noncontiguous reverse selection deletion");
        for (std::size_t i = 0; i < f.ids.size(); ++i)
            Require(f.scene.Objects()[i].id == f.ids[i], "Original order restored regardless of selection order");
        Require(history.DuplicateSelection(f.scene, f.selection) && !history.CanRedo(), "New group edit discards redo branch");
        const auto reopened = SandboxScene{DeserializeSceneDocument(SerializeSceneDocument(f.scene.ToDocument()))};
        Require(reopened.Objects().size() == f.scene.Objects().size(), "Group changes survive serialization");
        for (const auto& object : f.scene.Objects())
            Require(reopened.FindObject(object.id) && reopened.IsObjectVisible(object.id) == f.scene.IsObjectVisible(object.id),
                "Reload keeps IDs and visibility");
        auto invalid = f.selection;
        invalid.AddObject(99999);
        const auto before = SerializeSceneDocument(f.scene.ToDocument());
        Require(!history.DuplicateSelection(f.scene, invalid) && !history.DeleteSelection(f.scene, invalid) &&
            before == SerializeSceneDocument(f.scene.ToDocument()), "Invalid member prevents partial group changes");
    }
    void GroupTransformsAndFocus()
    {
        Fixture f;
        SceneEditHistory history;
        const auto active = *f.selection.SelectedObjectId();
        const auto before = f.scene.FindObject(active)->transform;
        auto after = before; after.position += glm::vec3{1,2,3};
        auto batch = BuildObjectTransformEdits(f.scene, f.selection, before, after, GizmoOperation::Translate);
        Require(batch && history.ApplyObjectBatch(f.scene, *batch), "Group translation applied");
        Require(Near(f.scene.FindObject(f.ids[0])->transform.position,{1,2,3}) &&
            Near(f.scene.FindObject(active)->transform.position,{7,2,3}) &&
            Near(f.scene.FindObject(f.ids[1])->transform.position,{3,0,0}), "Translation keeps spacing and leaves unselected object alone");
        Require(history.Undo(f.scene) && history.Redo(f.scene) && history.Undo(f.scene), "One-step group transform undo/redo");
        after = before; after.rotationRadians.y = 1.0f;
        batch = BuildObjectTransformEdits(f.scene, f.selection, before, after, GizmoOperation::Rotate);
        Require(batch && history.ApplyObjectBatch(f.scene, *batch), "Group rotation applied");
        Require(Near(f.scene.FindObject(f.ids[0])->transform.rotationRadians, {0,1,0}) &&
            Near(f.scene.FindObject(active)->transform.position, before.position), "Rotation uses individual pivots");
        Require(history.Undo(f.scene), "Undo rotation");
        after = before; after.scale = {2,3,4};
        f.scene.FindObject(f.ids[0])->transform.scale = {-2,1,0.5f};
        batch = BuildObjectTransformEdits(f.scene, f.selection, before, after, GizmoOperation::Scale);
        Require(batch && history.ApplyObjectBatch(f.scene, *batch) &&
            Near(f.scene.FindObject(f.ids[0])->transform.scale, {-4,3,2}), "Scale uses ratios, preserving negative/nonuniform scales");
        const auto bounds = SelectionFocusBounds(f.scene.Objects(), {f.scene.MainDirectionalLight(), f.scene.PointLights(), f.scene.SpotLights()}, f.selection, {{-.5f,-.5f,-.5f},{.5f,.5f,.5f}});
        Require(bounds && bounds->minimum.x <= -2 && bounds->maximum.x >= 7, "Focus encloses all selected bounds");
        auto singular = before; singular.scale.x = 0;
        Require(!BuildObjectTransformEdits(f.scene, f.selection, before, singular, GizmoOperation::Scale), "Reject singular group scale");
        auto nonfinite = before; nonfinite.position.x = std::numeric_limits<float>::infinity();
        Require(!BuildObjectTransformEdits(f.scene, f.selection, before, nonfinite, GizmoOperation::Translate), "Reject nonfinite group transform");
        ObjectBatchEdit visibility;
        for (const auto id : f.selection.SelectedObjectIds())
            visibility.edits.emplace_back(ObjectVisibilityEdit{id, f.scene.IsObjectVisible(id), false});
        Require(history.ApplyObjectBatch(f.scene, visibility) && !f.scene.IsObjectVisible(f.ids[0]), "Hide group");
        Require(history.Undo(f.scene) && f.scene.IsObjectVisible(f.ids[0]) && !f.scene.IsObjectVisible(active), "Undo restores mixed visibility");
        f.selection.AddObject(99999);
        ClearMissingSceneSelection(f.selection, f.scene);
        Require(f.selection.SelectedObjectIds().size() == 2 && f.selection.SelectedObjectId() == active, "Prune stale members without losing valid selection");
    }
    void EditorGroupDragAndPivots()
    {
        Fixture f;
        SceneTransformEditor editor;
        const glm::mat4 matrix{1};
        const auto frame = [&]
        {
            editor.BeginFrame();
            editor.Update({.scene = f.scene, .selection = f.selection, .view = matrix,
                .projection = matrix, .viewportHovered = true});
        };
        gizmoTranslation = {1,0,0}; gizmoChanged = true; gizmoActive = true;
        frame();
        f.selection.SelectObject(f.ids[1]); // Original drag targets must stay captured.
        frame();
        gizmoChanged = false; gizmoActive = false;
        frame(); frame();
        Require(Near(f.scene.FindObject(f.ids[0])->transform.position,{2,0,0}) &&
            Near(f.scene.FindObject(f.ids[2])->transform.position,{8,0,0}) &&
            Near(f.scene.FindObject(f.ids[1])->transform.position,{3,0,0}), "Live drag edits captured group even after selection changes");
        Require(editor.Undo(f.scene, &f.selection) && Near(f.scene.FindObject(f.ids[0])->transform.position,{0,0,0}) &&
            Near(f.scene.FindObject(f.ids[2])->transform.position,{6,0,0}) && !editor.CanUndo(), "Whole drag produces one undo entry");
        Require(editor.Redo(f.scene, &f.selection) && Near(f.scene.FindObject(f.ids[2])->transform.position,{8,0,0}), "Drag redo restores all members");
        f.selection.Clear(); f.selection.AddObject(f.ids[0]); f.selection.AddObject(f.ids[2]);
        f.scene.FindObject(f.ids[0])->localBounds = Renderer::AxisAlignedBoundingBox{{1,2,3},{3,4,5}};
        f.scene.FindObject(f.ids[2])->localBounds = Renderer::AxisAlignedBoundingBox{{-4,-2,-6},{0,2,-2}};
        const auto firstMatrix = f.scene.FindObject(f.ids[0])->transform.ModelMatrix();
        const auto secondMatrix = f.scene.FindObject(f.ids[2])->transform.ModelMatrix();
        Require(editor.SetSelectedObjectPivots(f.scene, f.selection, {{-1,-1,-1},{1,1,1}}, true), "Group bounds-center pivots");
        Require(Near(f.scene.FindObject(f.ids[0])->transform.pivot,{2,3,4}) &&
            Near(f.scene.FindObject(f.ids[2])->transform.pivot,{-2,0,-4}) &&
            f.scene.FindObject(f.ids[0])->transform.ModelMatrix() == firstMatrix &&
            f.scene.FindObject(f.ids[2])->transform.ModelMatrix() == secondMatrix, "Each member uses own bounds without moving geometry");
        Require(editor.Undo(f.scene) && Near(f.scene.FindObject(f.ids[0])->transform.pivot,{0,0,0}) &&
            Near(f.scene.FindObject(f.ids[2])->transform.pivot,{0,0,0}), "One undo restores all pivots");
        Require(editor.SetSelectionVisible(f.scene, f.selection, false) && !f.scene.IsObjectVisible(f.ids[0]) &&
            editor.Undo(f.scene) && f.scene.IsObjectVisible(f.ids[0]) && !f.scene.IsObjectVisible(f.ids[2]),
            "Editor visibility action restores mixed member states with one undo");
    }
    void ViewportGestures()
    {
        Fixture f;
        Platform::InputState input;
        ViewportSelectionController picking;
        const auto view = glm::lookAt(glm::vec3{0,0,5}, glm::vec3{0,0,0}, glm::vec3{0,1,0});
        const auto projection = glm::perspective(glm::radians(60.0f), 1.0f, .1f, 100.0f);
        const Renderer::AxisAlignedBoundingBox bounds{{-.5f,-.5f,-.5f},{.5f,.5f,.5f}};
        const auto click = [&](glm::vec2 position, bool ctrl, bool shift, bool suppressed = false)
        {
            input.Clear(); input.SetMouseButtonState(Platform::MouseButton::Left, true);
            picking.Update(input, position, {100,100}, view, projection, bounds, f.scene.Objects(), {}, f.selection,
                suppressed, ctrl, shift);
        };
        f.selection.SelectObject(f.ids[1]);
        click({50,50}, false, true);
        Require(f.selection.IsObjectSelected(f.ids[0]) && f.selection.IsObjectSelected(f.ids[1]), "Scene Shift-click adds hit object");
        click({50,50}, true, false);
        Require(!f.selection.IsObjectSelected(f.ids[0]) && f.selection.IsObjectSelected(f.ids[1]), "Scene Ctrl-click toggles hit off");
        click({0,0}, true, false);
        Require(f.selection.IsObjectSelected(f.ids[1]), "Modified empty-space click preserves group");
        click({50,50}, false, false, true);
        Require(f.selection.IsObjectSelected(f.ids[1]), "Suppressed picking cannot mutate selection");
        click({0,0}, false, false);
        Require(f.selection.Empty(), "Plain empty-space click clears group");
    }
}
int main()
{
    try { SelectionGestures(); BatchHistory(); GroupTransformsAndFocus(); EditorGroupDragAndPivots(); ViewportGestures(); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    std::cout << "Object multi-selection tests passed\n";
}
