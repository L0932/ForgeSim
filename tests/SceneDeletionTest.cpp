#include "EditorSession.hpp"
#include "SceneEditHistory.hpp"
#include "SceneSerializer.hpp"
#include "SceneDocumentIO.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    { if (!condition) throw std::runtime_error(message); }
    struct TemporaryDirectory
    {
        fs::path path = fs::temp_directory_path() / ("ForgeSimDeletion-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        TemporaryDirectory() { fs::create_directories(path); }
        ~TemporaryDirectory() { std::error_code error; fs::remove_all(path, error); }
    };
    void TestCubeCreation(const fs::path& root)
    {
        EditorSession session(SceneDocument{}, root);
        SceneSelection selection;
        SceneEditHistory history;
        auto& scene = session.Scene();
        Require(history.CreateCube(scene, selection, {0,2,0}), "Create cube in empty scene");
        const auto id = *selection.SelectedObjectId();
        Require(scene.Objects().size() == 1 && session.HasUnsavedChanges(), "Cube creation marks scene dirty");
        Require(scene.FindObject(id)->transform.position.y == 2 && scene.FindObject(id)->localBounds.has_value(), "Cube has placement and picking bounds");
        const auto path = root / "cube.scene.json";
        SaveSceneDocument(scene.ToDocument(), path, root);
        const auto reloaded = LoadSceneDocument(path, root);
        Require(reloaded.objects.size() == 1 && reloaded.objects[0].modelPath == "@builtin/cube", "Cube reload needs no external model file");
        Require(history.Undo(scene, &selection) && scene.Objects().empty() && selection.Empty(), "Undo cube creation");
        Require(!session.HasUnsavedChanges(), "Undo restores clean scene");
        Require(history.Redo(scene, &selection) && selection.IsObjectSelected(id), "Redo restores and selects same cube");
        Require(history.DeleteSelection(scene, selection) && history.Undo(scene, &selection), "Cube deletion is undoable");
        Require(history.CreateCube(scene, selection, {1,2,3}) && *selection.SelectedObjectId() != id, "Further cubes get unique IDs");
    }

    void TestObjectDeletion(const fs::path& root)
    {
        SceneDocument document;
        for (SandboxObjectId id : {1u, 2u, 3u})
            document.objects.push_back({.id = id, .name = "Model " + std::to_string(id), .transform = {}, .modelPath = "model.glb"});
        document.hiddenObjectIds = {2};
        document.objects[1].transform.position = {5,6,7};
        document.objects[1].transform.pivot = {1,2,3};
        EditorSession session(document, root);
        auto& scene = session.Scene();
        scene.Objects()[1].localBounds = ForgeSim::Renderer::AxisAlignedBoundingBox{{-2,-3,-4}, {2,3,4}};
        const auto original = SerializeSceneDocument(scene.ToDocument());
        SceneSelection selection;
        SceneEditHistory history;
        selection.SelectObject(2);
        Require(history.DeleteSelection(scene, selection) && selection.Empty() && !scene.FindObject(2),
            "Deletion must remove the selected model and clear selection");
        Require(session.HasUnsavedChanges() && !scene.HasHiddenObjects(),
            "Deletion must mark scene dirty and remove obsolete visibility IDs");
        Require(history.Undo(scene, &selection) && selection.IsObjectSelected(2), "Undo must select restored model");
        Require(scene.Objects()[1].id == 2 && !scene.IsObjectVisible(2) && scene.FindObject(2)->localBounds &&
            scene.FindObject(2)->localBounds->maximum.z == 4, "Undo must restore list position, visibility, and runtime model bounds");
        Require(SerializeSceneDocument(scene.ToDocument()) == original && !session.HasUnsavedChanges(),
            "Undo must restore all authored model properties and the clean baseline");
        Require(history.Redo(scene, &selection) && selection.Empty(), "Redo must remove the model again");
        session.SaveAs(root / "deleted.scene.json");
        const auto reopened = session.PrepareOpen(root / "deleted.scene.json", root);
        Require(reopened.Scene().Objects().size() == 2 && !reopened.Scene().FindObject(2),
            "Saved deletion must survive reopen without invalid visibility references");

        // Deleted IDs must not be reused by a later import while undo is available.
        selection.SelectObject(3);
        Require(history.DeleteSelection(scene, selection), "Delete highest model ID");
        auto imported = session.PrepareImportModel(root / "model.glb");
        Require(imported.Scene().Objects().back().id == 4, "Import must not reuse an ID retained by deletion history");
        session = std::move(imported);
        Require(history.Undo(scene, &selection) && scene.FindObject(3) && scene.FindObject(4),
            "Undo deletion after importing must preserve both independent objects");
        selection.SelectObject(99);
        Require(!history.DeleteSelection(scene, selection) && history.CanRedo(), "No-op deletion must preserve redo history");
        selection.SelectObject(1);
        Require(history.DeleteSelection(scene, selection) && !history.CanRedo(), "New deletion must discard redo branch");
    }
    void TestLightDeletion(const fs::path& root)
    {
        EditorSession session(SceneDocument{}, root);
        auto& scene = session.Scene();
        const auto point = scene.AddPointLight();
        const auto spot = scene.AddSpotLight();
        scene.PointLights().front().range = 42;
        scene.PointLights().front().enabled = false;
        scene.SpotLights().front().intensity = 7;
        scene.SetLightVisible(point, false);
        scene.SetLightVisible(spot, false);
        session.ResetChangeBaseline();
        const auto original = SerializeSceneDocument(scene.ToDocument());
        SceneSelection selection;
        SceneEditHistory history;
        selection.SelectLight(point);
        Require(history.DeleteSelection(scene, selection) && scene.PointLights().empty(), "Delete point light");
        selection.SelectLight(spot);
        Require(history.DeleteSelection(scene, selection) && scene.SpotLights().empty() && !scene.HasHiddenLights(),
            "Delete spot light and remove hidden light IDs");
        Require(history.Undo(scene, &selection) && selection.IsLightSelected(spot) && !scene.IsLightVisible(spot), "Undo spot deletion");
        Require(history.Undo(scene, &selection) && selection.IsLightSelected(point) && scene.PointLights().front().range == 42,
            "Undo point deletion must restore properties and selection");
        Require(SerializeSceneDocument(scene.ToDocument()) == original && !session.HasUnsavedChanges(),
            "Undo light deletions must restore exact authored state");
        Require(history.Redo(scene, &selection) && history.Redo(scene, &selection), "Redo both local-light deletions");
        session.SaveAs(root / "lights.scene.json");
        const auto loaded = session.PrepareOpen(root / "lights.scene.json", root);
        Require(loaded.Scene().PointLights().empty() && loaded.Scene().SpotLights().empty(), "Local-light deletion persists");
        selection.SelectLight(scene.MainDirectionalLight().id);
        const auto directionalId = scene.MainDirectionalLight().id;
        scene.MainDirectionalLight().enabled = false;
        scene.SetLightVisible(directionalId, false);
        Require(history.DeleteSelection(scene, selection) && !scene.HasDirectionalLight() &&
            !scene.HasHiddenLights() && selection.Empty(), "Delete must remove even a disabled/hidden directional light");
        selection.SelectLight(directionalId);
        Require(!history.DeleteSelection(scene, selection), "Deleted directional selection is a no-op");
        Require(history.Undo(scene, &selection) && scene.HasDirectionalLight() &&
            !scene.MainDirectionalLight().enabled && !scene.IsLightVisible(directionalId) && selection.IsLightSelected(directionalId),
            "Undo must restore directional properties, visibility, and selection");
        Require(history.Redo(scene, &selection) && selection.Empty() && !scene.HasDirectionalLight(),
            "Redo must remove directional light again");
        session.Save();
        const auto withoutDirectional = session.PrepareOpen(root / "lights.scene.json", root);
        Require(!withoutDirectional.Scene().HasDirectionalLight(), "Directional absence persists through save/reopen");
        Require(nlohmann::json::parse(SerializeSceneDocument(scene.ToDocument())).at("directionalLight").is_null(),
            "A deleted directional light must be saved as null, not as a disabled entity");
    }
    void TestMixedTransformAndDeletion(const fs::path& root)
    {
        SceneDocument document;
        document.objects.push_back({.id = 1, .name = "Model", .transform = {}, .modelPath = "model.glb"});
        EditorSession session(document, root);
        auto& scene = session.Scene();
        SceneEditHistory history;
        SceneSelection selection;
        const auto before = scene.Objects().front().transform;
        auto after = before;
        after.position = {3,4,5};
        scene.Objects().front().transform = after;
        history.RecordApplied(ObjectTransformEdit{1, before, after});
        selection.SelectObject(1);
        Require(history.DeleteSelection(scene, selection), "Delete after transform edit");
        Require(history.Undo(scene, &selection) && scene.FindObject(1)->transform.position == after.position,
            "Undo deletion must restore the transformed object");
        Require(history.Undo(scene, &selection) && scene.FindObject(1)->transform.position == before.position,
            "Next undo must restore the preceding transform");
        Require(history.Redo(scene, &selection) && scene.FindObject(1)->transform.position == after.position,
            "Redo transform before replaying deletion");
        Require(history.Redo(scene, &selection) && !scene.FindObject(1) && selection.Empty(),
            "Redo deletion after transform replay");
    }
    void TestLightCreation(const fs::path& root)
    {
        SceneDocument document;
        document.hasDirectionalLight = false;
        EditorSession session(document, root);
        auto& scene = session.Scene();
        SceneEditHistory history;
        SceneSelection selection;
        const auto baseline = SerializeSceneDocument(scene.ToDocument());
        const glm::vec3 position{3,4,5};
        const glm::vec3 direction{0,-1,0};
        Require(!scene.HasDirectionalLight(), "Empty directional slot stays absent");
        Require(history.CreateLight(scene, selection, SceneLightKind::Directional, position, direction, 12), "Create directional light");
        const auto directionalId = *selection.SelectedLightId();
        Require(scene.MainDirectionalLight().position == position && scene.MainDirectionalLight().directionToLight == -direction,
            "Directional creation must place its marker and aim along the supplied direction");
        Require(!history.CreateLight(scene, selection, SceneLightKind::Directional, position, direction, 12),
            "Single-directional renderer must reject a second directional without adding history");
        Require(history.CreateLight(scene, selection, SceneLightKind::Point, position, direction, 12), "Create point light");
        const auto pointId = *selection.SelectedLightId();
        Require(history.CreateLight(scene, selection, SceneLightKind::Spot, position, direction, 24), "Create spot light");
        const auto spotId = *selection.SelectedLightId();
        Require(directionalId != pointId && pointId != spotId && directionalId != spotId,
            "All light types must share unique scene IDs");
        Require(scene.PointLights().back().position == position && scene.PointLights().back().range == 12 &&
            scene.SpotLights().back().range == 24 && scene.SpotLights().back().emissionDirection == direction,
            "Created lights must retain placement, range, and orientation");
        Require(session.HasUnsavedChanges(), "Light creation must mark scene modified");
        for (int i = 0; i < 3; ++i) Require(history.Undo(scene, &selection), "Undo each light creation");
        Require(!scene.HasDirectionalLight() && scene.PointLights().empty() && scene.SpotLights().empty() &&
            SerializeSceneDocument(scene.ToDocument()) == baseline && !session.HasUnsavedChanges(),
            "Undoing light creation must restore the exact empty/clean scene");
        bool failed = false;
        try { static_cast<void>(history.CreateLight(scene, selection, SceneLightKind::Point,
            {std::numeric_limits<float>::quiet_NaN(),0,0}, direction, 12)); }
        catch (const std::invalid_argument&) { failed = true; }
        Require(failed && history.CanRedo() && SerializeSceneDocument(scene.ToDocument()) == baseline,
            "Failed creation must preserve scene and redo history");
        for (int i = 0; i < 3; ++i) Require(history.Redo(scene, &selection), "Redo each light creation");
        Require(selection.IsLightSelected(spotId) && scene.MainDirectionalLight().id == directionalId &&
            scene.PointLights().front().id == pointId && scene.SpotLights().front().id == spotId,
            "Redo creation must restore original IDs and select the recreated light");
        session.SaveAs(root / "created-lights.scene.json");
        const auto reopened = session.PrepareOpen(root / "created-lights.scene.json", root);
        Require(SerializeSceneDocument(reopened.Scene().ToDocument()) == SerializeSceneDocument(scene.ToDocument()),
            "Created lights must survive save/reopen with all authored properties");
        selection.SelectLight(directionalId);
        Require(history.DeleteSelection(scene, selection), "Remove original directional light");
        Require(history.CreateLight(scene, selection, SceneLightKind::Directional, position, direction, 12), "Recreate directional light");
        const auto replacementId = *selection.SelectedLightId();
        Require(replacementId != directionalId, "Recreated directional must not reuse a deleted ID retained by undo");
        Require(history.Undo(scene, &selection) && !scene.HasDirectionalLight(), "Undo recreation removes replacement");
        Require(history.Undo(scene, &selection) && scene.MainDirectionalLight().id == directionalId, "Undo deletion restores original");
        Require(history.Redo(scene, &selection) && history.Redo(scene, &selection) &&
            scene.MainDirectionalLight().id == replacementId, "Replay deletion and recreation in order");

        const auto legacy = DeserializeSceneDocument(SerializeSceneDocument(SceneDocument{}));
        Require(legacy.hasDirectionalLight && SandboxScene(legacy).HasDirectionalLight(), "Existing directional scene files remain supported");
        auto invalid = document;
        invalid.hiddenLightIds = {1};
        failed = false;
        try { static_cast<void>(SerializeSceneDocument(invalid)); } catch (const std::invalid_argument&) { failed = true; }
        Require(failed, "Visibility must not reference an absent directional light");
    }
    void TestRootRetention(const fs::path& root)
    {
        TemporaryDirectory outside;
        std::ofstream(outside.path / "other.glb") << "placeholder";
        EditorSession session(SceneDocument{}, root);
        session = session.PrepareImportModel(root / "model.glb");
        SceneEditHistory history;
        SceneSelection selection;
        selection.SelectObject(session.Scene().Objects().front().id);
        Require(history.DeleteSelection(session.Scene(), selection) && session.Scene().Objects().empty(), "Delete sole imported object");
        bool failed = false;
        try { static_cast<void>(session.PrepareImportModel(outside.path / "other.glb")); }
        catch (const std::invalid_argument&) { failed = true; }
        Require(failed, "Deleting the last model must not silently change roots while undo references the original assets");
        Require(history.Undo(session.Scene(), &selection), "Original model remains undoable after rejected outside-root import");
    }
}
int main()
{
    try
    {
        TemporaryDirectory temporary;
        const auto root = temporary.path;
        std::ofstream(root / "model.glb") << "placeholder";
        TestCubeCreation(root);
        TestObjectDeletion(root);
        TestLightDeletion(root);
        TestRootRetention(root);
        TestMixedTransformAndDeletion(root);
        TestLightCreation(root);
        std::cout << "Light creation/deletion, undo/redo, visibility, IDs, root retention, and persistence passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
