#include "DisplacedGeometry.hpp"
#include "HeightGlbFixture.hpp"
#include "SceneSerializer.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneTransformEditor.hpp"
#include "ImportHeightImage.hpp"
#include "ModelNodeQuery.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>

static_assert(std::is_nothrow_move_constructible_v<ForgeSim::Sandbox::SandboxObject>);
static_assert(std::is_nothrow_move_assignable_v<ForgeSim::Sandbox::SandboxObject>);
static_assert(std::is_nothrow_move_constructible_v<ForgeSim::Sandbox::SceneEdit>);
static_assert(std::is_nothrow_move_assignable_v<ForgeSim::Sandbox::SceneEdit>);
namespace fs = std::filesystem;
using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace ForgeSim::Sandbox
{
    void ViewportTransformGizmo::BeginFrame() {}
    GizmoInteraction ViewportTransformGizmo::Draw(const ViewportTransformGizmoContext&) { return {}; }
}
namespace
{
    void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
    template<typename Action> void Reject(Action action, const char* message)
    {
        bool failed = false; try { action(); } catch (const std::exception&) { failed = true; }
        Require(failed,message);
    }
    MaterialDisplacement Height()
    {
        MaterialDisplacement value; value.enabled = true; value.imageIndex = 0;
        value.scale = .2f; value.midlevel = .25f; value.subdivisions = 1;
        return value;
    }
    void OverrideStorage()
    {
        MaterialDisplacements values{{9,Height()},{2,MaterialDisplacement{}},{5,Height()}};
        Require(values.begin()->first == 2 && values.size() == 3,"Sparse overrides must stay sorted");
        Require(!values.emplace(5,MaterialDisplacement{}).second && values[5].enabled,"Duplicate insertion must retain existing value");
        auto copy = values; copy[5].scale = .7f;
        Require(values[5].scale != copy[5].scale,"Copied overrides must be independent");
        auto moved = std::move(copy);
        Require(moved.size() == 3 && moved[5].scale == .7f,"Move construction preserves settings");
        MaterialDisplacements assigned; assigned = std::move(moved);
        Require(assigned.erase(2) == 1 && assigned.erase(2) == 0,"Erase reports sparse key presence");
        Require(assigned.find(2) == assigned.end() && std::as_const(assigned).find(5) != assigned.end(),"Const and mutable lookup agree");
        assigned.swap(values);
        Require(assigned.size() == 3 && values.size() == 2,"Swap transfers complete sets without allocation");
        assigned.clear(); Require(assigned.empty(),"Clear removes overrides");
    }
    void NativeHeightAndGeometry()
    {
        Test::Fixture fixture;
        const auto path = fixture.Write("model.glb");
        auto source = Assets::LoadGltfScene(path);
        Require(source.textureResources.imageSources[0].name == "Wall_disp" &&
            source.textureResources.imageSources[0].embeddedPng,"Expose embedded height choices");
        const auto height = Assets::LoadGltfHeightPng(path,0);
        Require(height.samples[1] > height.samples[0] && height.samples[1]-height.samples[0] < 1.f/255,
            "On-demand embedded heights must retain native 16-bit precision");
        auto extra = source.meshes[0].primitives[0]; extra.materialIndex.reset();
        source.meshes[0].primitives.push_back(extra);
        const MaterialDisplacements values{{0,Height()}};
        const auto changed = BuildDisplacedGeometry(source,path,fixture.directory,values);
        const auto& mesh = changed.meshes[0].primitives[0].geometry;
        Require(mesh.indices.size()==12 && mesh.vertices[0].position[1] > .04f,"Apply real geometry height to selected material");
        Require(changed.meshes[0].primitives[1].geometry.vertices.size()==3 &&
            changed.meshes[0].primitives[1].geometry.vertices[0].position[1]==0,"Other materials must remain unchanged");
        Require(source.meshes[0].primitives[0].geometry.vertices.size()==3 &&
            source.meshes[0].primitives[0].geometry.vertices[0].position[1]==0,"Shared source geometry must remain unchanged");
        Require(changed.textureResources.images.empty(),"Geometry variants must borrow, rather than duplicate, color textures");
        const auto repeated = BuildDisplacedGeometry(source,path,fixture.directory,values);
        Require(repeated.meshes[0].primitives[0].geometry.vertices[0].position==mesh.vertices[0].position,"Apply must never accumulate offsets");
        const ModelNodeQuery query{changed};
        Require(query.ModelBounds({})->maximum.y > 3,"Bounds must include displaced geometry and node transform");
        auto mapped=values; mapped[0].uvScale={2,2};
        const auto withMapping=BuildDisplacedGeometry(source,path,fixture.directory,mapped);
        Require(withMapping.meshes[0].primitives[0].geometry.vertices[0].position[1] != mesh.vertices[0].position[1],
            "Editor height mapping must reach CPU displacement");
        Require(withMapping.meshes[0].primitives[0].geometry.vertices[0].textureCoordinates == source.meshes[0].primitives[0].geometry.vertices[0].textureCoordinates,
            "Editor height mapping must not retile color/normal maps");
        auto baked = source; baked.heightGeometryBaked = true;
        Reject([&]{static_cast<void>(BuildDisplacedGeometry(baked,path,fixture.directory,values));},"Reject prototype double displacement");
        auto disabled = values; disabled[0].enabled = false;
        Require(BuildDisplacedGeometry(source,path,fixture.directory,disabled).meshes[0].primitives[0].geometry.vertices.size()==3,
            "Disabling must restore original topology");
        auto bad = values; bad[0].imageIndex = 99;
        Reject([&]{static_cast<void>(BuildDisplacedGeometry(source,path,fixture.directory,bad));},"Reject missing embedded image");
        bad=values; bad[0].channel = 1;
        Reject([&]{static_cast<void>(BuildDisplacedGeometry(source,path,fixture.directory,bad));},"Reject absent height channel");
        bad=values; bad[0].subdivisions=8;
        auto oversized = source;
        for (unsigned i = 0; i < 8; ++i) oversized.meshes[0].primitives.push_back(source.meshes[0].primitives[0]);
        Reject([&]{static_cast<void>(BuildDisplacedGeometry(oversized,path,fixture.directory,bad));},"Reject oversized geometry");
        Require(source.meshes[0].primitives[0].geometry.vertices.size()==3,"Failed edits must preserve source mesh");
    }
    void PersistenceAndHistory()
    {
        SandboxScene scene;
        scene.Objects().front().modelPath="models/wall.glb";
        const auto id = scene.Objects().front().id;
        SceneTransformEditor editor;
        auto value = Height(); value.inverted = true; value.stitchSeams = false;
        value.wrapU = HeightWrap::ClampToEdge; value.wrapV = HeightWrap::MirroredRepeat;
        value.uvScale={2,3}; value.uvOffset={.2f,-1}; value.uvRotationRadians=.6f;
        const MaterialDisplacements settings{{0,value}};
        Require(editor.SetDisplacement(scene.Objects().front(),settings),"Apply editor settings");
        const auto document = scene.ToDocument();
        const auto text = SerializeSceneDocument(document);
        const auto loaded = DeserializeSceneDocument(text);
        Require(loaded.version == 6 && loaded.objects[0].displacement==settings,"Round trip every height setting");
        SandboxScene restored{loaded};
        Require(restored.Objects()[0].displacement==settings,"Restore object state from scene");
        Require(editor.Undo(scene) && scene.FindObject(id)->displacement.empty(),"Undo restores original settings");
        Require(editor.Redo(scene) && scene.FindObject(id)->displacement==settings,"Redo restores height settings");
        SceneSelection selection; selection.SelectObject(id);
        Require(editor.DuplicateSelection(scene,selection),"Duplicate displaced model");
        const auto copy = *selection.SelectedObjectId();
        Require(scene.FindObject(copy)->displacement==settings,"Duplicate retains displacement");
        Require(editor.SetDisplacement(*scene.FindObject(copy),{}),"Reset duplicate only");
        Require(scene.FindObject(id)->displacement==settings,"Another instance must remain unchanged");
        auto legacy=nlohmann::json::parse(text); legacy["version"]=5; legacy["objects"][0].erase("displacement");
        Require(DeserializeSceneDocument(legacy.dump()).objects[0].displacement.empty(),"Older scenes default to no displacement");
        auto oldHeight=nlohmann::json::parse(text);
        auto& oldEntry=oldHeight["objects"][0]["displacement"][0];
        oldEntry.erase("uvScale"); oldEntry.erase("uvOffset"); oldEntry.erase("uvRotationRadians");
        const auto oldSettings=DeserializeSceneDocument(oldHeight.dump()).objects[0].displacement[0];
        Require(oldSettings.uvScale==std::array<float,2>{1,1} && oldSettings.uvOffset==std::array<float,2>{0,0} && oldSettings.uvRotationRadians==0,
            "Existing height overrides must retain identity UV mapping");
        auto invalid=nlohmann::json::parse(text); invalid["objects"][0]["displacement"][0]["subdivisions"]=9;
        Reject([&]{static_cast<void>(DeserializeSceneDocument(invalid.dump()));},"Reject unsafe persisted detail");
        invalid=nlohmann::json::parse(text); invalid["objects"][0]["displacement"][0]["heightAsset"]="../height.png";
        Reject([&]{static_cast<void>(DeserializeSceneDocument(invalid.dump()));},"Reject nonportable or conflicting height assignment");
        invalid=nlohmann::json::parse(text); invalid["objects"][0]["displacement"][0]["uvScale"]={1,2,3};
        Reject([&]{static_cast<void>(DeserializeSceneDocument(invalid.dump()));},"Reject malformed saved UV pair");
        invalid=nlohmann::json::parse(text); invalid["objects"][0]["displacement"].push_back(invalid["objects"][0]["displacement"][0]);
        Reject([&]{static_cast<void>(DeserializeSceneDocument(invalid.dump()));},"Reject duplicate material assignment");
    }
    void ExternalHeightAndMissingAsset()
    {
        Test::Fixture fixture;
        const auto path = fixture.Write("model.glb");
        const auto source=Assets::LoadGltfScene(path);
        const auto& image = fixture.json["images"][0];
        const auto& view=fixture.json["bufferViews"][image["bufferView"].get<std::size_t>()];
        const auto external=fixture.directory/"height.png";
        { std::ofstream file{external,std::ios::binary};
          file.write(reinterpret_cast<const char*>(fixture.binary.data()+view["byteOffset"].get<std::size_t>()),view["byteLength"].get<std::streamsize>()); }
        MaterialDisplacement height=Height(); height.imageIndex.reset(); height.heightAsset="height.png";
        const auto changed=BuildDisplacedGeometry(source,path,fixture.directory,{{0,height}});
        Require(changed.meshes[0].primitives[0].geometry.vertices[0].position[1]>.04f,"External PNG must displace geometry");
        const auto project=fixture.directory/"project"; fs::create_directories(project);
        std::string imported;
        ImportHeightImage(external,project,[&](const std::string& reference){ imported=reference; });
        Require(imported=="textures/heights/height.png" && fs::exists(project/imported),"Copy external PNG to portable project location");
        Reject([&]{ImportHeightImage(external,project,[&](const std::string&){throw std::runtime_error("cancel");});},"Failed import should propagate");
        Require(!fs::exists(project/"textures/heights/height_1.png"),"Failed import removes only its new copy");
        SceneDocument document; document.objects.push_back({.id=1,.name="Wall",.modelPath="model.glb",.displacement={{0,height}}});
        const auto saved=fixture.directory/"wall.scene.json";
        SaveSceneDocument(document,saved,fixture.directory);
        fs::remove(external);
        bool missingHeight=false;
        try { static_cast<void>(LoadSceneDocument(saved,fixture.directory)); }
        catch(const MissingSceneAssets& missing)
        { missingHeight=!missing.assets.empty() && missing.assets[0].field.ends_with(".heightAsset"); }
        Require(missingHeight,"Missing external height must participate in scene asset recovery");
    }
}
int main()
{
    try { OverrideStorage(); NativeHeightAndGeometry(); PersistenceAndHistory(); ExternalHeightAndMissingAsset(); }
    catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
    std::cout<<"Material displacement precision, isolation, persistence, history, portable images and recovery passed\n";
}
