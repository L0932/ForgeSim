#include "EditorSession.hpp"
#include "SceneEditHistory.hpp"
#include "SceneSerializer.hpp"
#include "SceneDocumentIO.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

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
        Require(history.DeleteSelection(scene, selection) && !scene.MainDirectionalLight().enabled &&
            scene.MainDirectionalLight().id == directionalId && selection.Empty(), "Delete disables required directional singleton");
        selection.SelectLight(directionalId);
        Require(!history.DeleteSelection(scene, selection), "Already-disabled directional light must not add history");
        Require(history.Undo(scene, &selection) && scene.MainDirectionalLight().enabled && selection.IsLightSelected(directionalId),
            "Undo must restore directional enablement and selection");
        Require(history.Redo(scene, &selection) && selection.Empty() && !scene.MainDirectionalLight().enabled,
            "Redo must disable directional light again");
        session.Save();
        Require(!session.PrepareOpen(root / "lights.scene.json", root).Scene().MainDirectionalLight().enabled,
            "Disabled directional light persists through save/reopen");
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
        TestObjectDeletion(root);
        TestLightDeletion(root);
        TestRootRetention(root);
        TestMixedTransformAndDeletion(root);
        std::cout << "Deletion, undo/redo, visibility, IDs, root retention, and persistence passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
