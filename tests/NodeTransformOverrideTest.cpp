#include "ModelNodeQuery.hpp"
#include "SceneTransformEditor.hpp"
#include "SceneSerializer.hpp"
#include "SceneDocumentValidation.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <nlohmann/json.hpp>
#include <glm/trigonometric.hpp>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace ForgeSim::Sandbox
{
    void ViewportTransformGizmo::BeginFrame() {}
    GizmoInteraction ViewportTransformGizmo::Draw(const ViewportTransformGizmoContext&) { return {}; }
}
static_assert(std::is_nothrow_move_constructible_v<SandboxObject>);
static_assert(std::is_nothrow_move_assignable_v<SandboxObject>);
static_assert(std::is_nothrow_move_constructible_v<SceneEdit>);
namespace
{
    void Require(bool condition, const char* message) { if(!condition) throw std::runtime_error(message); }
    bool Near(glm::vec3 a,glm::vec3 b) { return glm::length(a-b)<.0001f; }
    Assets::SceneAsset Asset()
    {
        Assets::SceneAsset asset; asset.nodes.resize(4); asset.rootNodeIndices={0};
        asset.nodes[0].childIndices={1,2}; asset.nodes[0].localTransform.translation={10,0,0};
        asset.nodes[1].meshIndex=0;asset.nodes[2].meshIndex=0;
        asset.nodes[2].localTransform.translation={4,0,0};
        // Node 3 deliberately lies outside the active scene.
        asset.meshes.resize(1);Assets::MeshPrimitiveData primitive;
        primitive.geometry.vertices.resize(3);
        primitive.geometry.vertices[0].position={0,0,0};
        primitive.geometry.vertices[1].position={1,0,0};
        primitive.geometry.vertices[2].position={0,1,0};
        primitive.geometry.indices={0,1,2}; asset.meshes[0].primitives.push_back(primitive);
        return asset;
    }
    void StorageAndMoves()
    {
        NodeTransformOverrides overrides;
        Transform first; first.position={1,2,3};
        Transform second; second.position={4,5,6};
        overrides.emplace(9,first); overrides.emplace(2,second); overrides[5]={};
        Require(overrides.size()==3 && overrides.begin()->first==2,"Sparse storage iterates in canonical index order");
        Require(!overrides.emplace(9,second).second && Near(overrides.at(9).position,first.position),"Duplicate insertion preserves existing value");
        overrides.insert_or_assign(9,second);
        const auto copy=overrides;
        NodeTransformOverrides moved=std::move(overrides);
        NodeTransformOverrides assigned; assigned[123]={}; assigned=std::move(moved);
        Require(assigned.size()==3 && !assigned.contains(123) && Near(assigned.at(9).position,second.position),"Move construction and assignment retain values and replace old storage");
        assigned.at(2).position={99,99,99};
        Require(Near(copy.at(2).position,second.position),"Copies retain independent value ownership");
        Require(assigned.erase(5)==1 && assigned.erase(5)==0 && !assigned.contains(5),"Erase handles present and missing keys");
        assigned.clear();Require(assigned.empty(),"Clear removes all overrides");
    }
    void Geometry()
    {
        auto asset=Asset();ModelNodeQuery query(asset);NodeTransformOverrides overrides;
        Transform adjustment;adjustment.position={0,3,0};overrides[1]=adjustment;
        const auto moved=query.WorldBounds(1,glm::mat4{1},overrides);
        Require(moved && Near(moved->minimum,{10,3,0}),"Node offset affects its own geometry");
        const auto sibling=query.WorldBounds(2,glm::mat4{1},overrides);
        Require(sibling && Near(sibling->minimum,{14,0,0}),"Sibling remains unchanged");
        const auto whole=query.ModelBounds(overrides);
        Require(whole && Near(whole->maximum,{15,4,0}),"Whole-instance bounds include edited geometry");
        Require(query.Pick(asset,{{10.2f,3.2f,5},{0,0,-1}},glm::mat4{1},overrides)->nodeIndex==1,
            "Picking follows moved geometry");
        Require(!query.Pick(asset,{{10.2f,.2f,5},{0,0,-1}},glm::mat4{1},overrides),"Old position no longer picks");
        Require(Near(query.WorldBounds(1,glm::mat4{1})->minimum,{10,0,0}),"Shared asset and unedited instances remain unchanged");
        adjustment.position={0,0,2};adjustment.rotationRadians.z=glm::radians(90.f);adjustment.scale={2,2,2};overrides[0]=adjustment;
        const auto matrices=query.Evaluate(overrides);
        Require(Near(glm::vec3{matrices[1]*glm::vec4{0,0,0,1}},{4,0,2}),"Parent rotation and scale transform the descendant offset");
        Require(Near(glm::vec3{matrices[2]*glm::vec4{0,0,0,1}},{10,8,2}),"Parent adjustment reaches sibling descendants");
        const auto instanced=query.WorldBounds(1,glm::translate(glm::mat4{1},glm::vec3{100,0,0}),overrides);
        Require(instanced && instanced->minimum.x>100,"Owner instance transform composes with node edits");
        overrides[3]={};bool rejected=false;
        try {static_cast<void>(query.ModelBounds(overrides));}catch(const std::invalid_argument&){rejected=true;}
        Require(rejected,"Overrides on unreachable asset nodes fail validation");
        // Matrix-authored nodes keep their full matrix; no lossy decomposition.
        asset.nodes[0].localTransform.matrix=std::array<float,16>{1,0,0,0, .5f,1,0,0, 0,0,1,0, 10,0,0,1};
        ModelNodeQuery matrixQuery(asset);overrides.clear();Transform offset;offset.position={0,2,0};overrides[0]=offset;
        Require(Near(glm::vec3{matrixQuery.Evaluate(overrides)[0]*glm::vec4{0,0,0,1}},{11,2,0}),"Matrix-authored shear is preserved");
    }
    void HistoryAndPersistence()
    {
        SandboxScene scene;auto& object=scene.Objects().front();object.modelPath="assets/model.glb";
        const auto id=object.id;SceneSelection selection;selection.SelectNode(id,1,object.modelPath);
        SceneTransformEditor editor;Transform value;value.position={2,3,4};value.scale={-1,2,3};
        Require(editor.SetNodeOverride(object,1,value),"Apply node adjustment");
        Require(editor.CanUndo() && object.nodeOverrides.contains(1),"Adjustment records history");
        Require(editor.Undo(scene,&selection) && scene.FindObject(id)->nodeOverrides.empty(),"Undo restores absence of override");
        Require(selection.SelectedNode() && selection.SelectedNode()->nodeIndex==1,"Undo retains node selection");
        Require(editor.Redo(scene,&selection) && Near(scene.FindObject(id)->nodeOverrides.at(1).position,{2,3,4}),"Redo restores adjustment");
        Require(editor.SetNodeOverride(*scene.FindObject(id),1,std::nullopt),"Reset removes override");
        Require(editor.Undo(scene) && scene.FindObject(id)->nodeOverrides.contains(1),"Reset is undoable");
        const auto serialized=SerializeSceneDocument(scene.ToDocument());
        const auto restored=DeserializeSceneDocument(serialized);
        Require(restored.version==5 && restored.objects.front().nodeOverrides.size()==1,"Scene schema persists node overrides");
        SandboxScene loaded(restored);Require(Near(loaded.Objects().front().nodeOverrides.at(1).scale,{-1,2,3}),"Round trip preserves mirrored scale");
        Require(SerializeSceneDocument(loaded.ToDocument())==serialized,"Canonical serialization stabilizes dirty tracking");
        auto old=nlohmann::json::parse(serialized);old["version"]=4;old["objects"][0].erase("nodeOverrides");
        Require(DeserializeSceneDocument(old.dump()).objects.front().nodeOverrides.empty(),"Version 4 files migrate with no overrides");
        auto duplicate=nlohmann::json::parse(serialized);duplicate["objects"][0]["nodeOverrides"].push_back(duplicate["objects"][0]["nodeOverrides"][0]);
        bool rejected=false;try{static_cast<void>(DeserializeSceneDocument(duplicate.dump()));}catch(const std::exception&){rejected=true;}
        Require(rejected,"Duplicate override indices are rejected");
        auto invalid=restored;invalid.objects[0].nodeOverrides.at(1).scale.x=0;
        Require(ValidateSceneDocument(invalid).has_value(),"Zero node scale is rejected on load");
        value.position.x=std::numeric_limits<float>::quiet_NaN();
        Require(!editor.SetNodeOverride(*scene.FindObject(id),1,value),"Nonfinite editing values are rejected");
        selection.SelectObject(id);Require(editor.DuplicateSelection(scene,selection),"Model with overrides duplicates");
        const auto copyId=*selection.SelectedObjectId();Require(scene.FindObject(copyId)->nodeOverrides.size()==1,"Duplicate owns a copy of node overrides");
        Transform copyEdit;copyEdit.position={9,9,9};Require(editor.SetNodeOverride(*scene.FindObject(copyId),1,copyEdit),"Edit duplicate");
        Require(Near(scene.FindObject(id)->nodeOverrides.at(1).position,{2,3,4}),"Duplicate edits leave original untouched");
        Require(editor.Undo(scene) && Near(scene.FindObject(copyId)->nodeOverrides.at(1).position,{2,3,4}),"Undo duplicate edit");
        Require(editor.Undo(scene,&selection) && !scene.FindObject(copyId),"Undo duplication");
        Require(editor.Redo(scene,&selection) && scene.FindObject(copyId)->nodeOverrides.size()==1,"Redo duplication preserves node state");
        Require(editor.DeleteSelection(scene,selection) && !scene.FindObject(copyId),"Delete model carrying node overrides");
        Require(editor.Undo(scene,&selection) && scene.FindObject(copyId)->nodeOverrides.size()==1 &&
            Near(scene.FindObject(copyId)->nodeOverrides.at(1).position,{2,3,4}),"Undo deletion restores node overrides through entity moves");
        Require(editor.Redo(scene,&selection) && !scene.FindObject(copyId),"Redo deletion retains valid history");
    }
}
int main()
{
    try {StorageAndMoves();Geometry();HistoryAndPersistence();}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"Node overrides: hierarchy, bounds, picking, instance isolation, history and persistence passed\n";
}
