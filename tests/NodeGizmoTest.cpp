#include "NodeGizmoTransform.hpp"
#include "SceneTransformEditor.hpp"
#include "SceneSerializer.hpp"
#include "SceneBounds.hpp"
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace
{
    glm::mat4 desired{1}; bool changed=false,active=false; int enabledDraws=0;
    GizmoSpace observedSpace=GizmoSpace::World;
}
namespace ForgeSim::Sandbox
{
    void ViewportTransformGizmo::BeginFrame() {}
    GizmoInteraction ViewportTransformGizmo::Draw(const ViewportTransformGizmoContext& context)
    {
        if (!context.enabled) return {};
        ++enabledDraws; observedSpace=context.space;
        if (changed) context.transform=desired;
        return {changed,active,active};
    }
}
namespace
{
    void Require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
    bool Near(glm::vec3 a,glm::vec3 b){return glm::length(a-b)<.0001f;}
    Assets::SceneAsset Asset()
    {
        Assets::SceneAsset asset;asset.nodes.resize(3);asset.rootNodeIndices={0};
        asset.nodes[0].childIndices={1,2};asset.nodes[0].localTransform.translation={10,0,0};
        const float half=glm::radians(45.f);
        asset.nodes[0].localTransform.rotation={0,0,std::sin(half),std::cos(half)};
        asset.nodes[1].meshIndex=0;asset.nodes[1].localTransform.translation={1,0,0};
        asset.nodes[2].meshIndex=0;asset.nodes[2].localTransform.translation={5,0,0};
        asset.meshes.resize(1);Assets::MeshPrimitiveData primitive;primitive.geometry.vertices.resize(3);
        primitive.geometry.vertices[0].position={0,0,0};primitive.geometry.vertices[1].position={1,0,0};
        primitive.geometry.vertices[2].position={0,1,0};asset.meshes[0].primitives.push_back(primitive);
        return asset;
    }
    struct Fixture
    {
        Assets::SceneAsset asset=Asset();ModelNodeQuery query{asset};SandboxScene scene;
        SceneSelection selection;SceneTransformEditor editor;glm::mat4 view{1},projection{1};
        SandboxObjectId id=scene.Objects().front().id;
        Fixture()
        {
            scene.Objects().front().modelPath="model.glb";
            scene.Objects().front().transform.position={100,0,0};
            scene.Objects().front().transform.scale={2,2,2};
            selection.SelectNode(id,1,"model.glb");changed=false;active=false;enabledDraws=0;
        }
        SandboxObject& Object(){return *scene.FindObject(id);}
        glm::mat4 Base(std::size_t index=1){return *NodeGizmoBase(query,index,Object().transform.ModelMatrix(),Object().nodeOverrides);}
        void Frame(GizmoOperation operation=GizmoOperation::Translate,GizmoSpace space=GizmoSpace::World,
            bool navigation=false,bool hovered=true)
        {
            editor.BeginFrame();
            editor.Update({.scene=scene,.selection=selection,.view=view,.projection=projection,
                .viewportSize={800,600},.viewportHovered=hovered,.operation=operation,.space=space,
                .navigationActive=navigation,.inspectNodes=[&](const std::string& path)
                {return path=="model.glb"? &query:nullptr;}});
        }
        void Release(GizmoOperation operation=GizmoOperation::Translate)
        {changed=false;active=false;Frame(operation);Frame(operation);}
    };
    void DragAndHistory()
    {
        Fixture f;const auto base=f.Base();const auto initial=f.scene.ToDocument();
        desired=base;desired[3]+=glm::vec4{0,4,0,0};changed=true;active=true;f.Frame();
        Require(f.editor.IsActive() && f.editor.SuppressesPicking(),"Node drag suppresses geometry picking");
        Require(Near(f.Object().nodeOverrides.at(1).position,{2,0,0}),"World translation maps through rotated scaled parent");
        Require(!f.editor.CanUndo(),"No undo during a drag");
        desired=base;desired[3]+=glm::vec4{0,8,0,0};f.Frame();
        f.Release();
        Require(Near(f.Object().nodeOverrides.at(1).position,{4,0,0}),"Continued drag updates same node");
        Require(f.Object().localBounds && f.Object().localBounds->maximum.y>5,"Live object bounds follow node gizmo");
        Require(f.editor.Undo(f.scene) && f.Object().nodeOverrides.empty(),"One undo restores entire multi-frame drag");
        Require(!f.editor.CanUndo(),"Drag creates one history entry");
        Require(f.editor.Redo(f.scene) && Near(f.Object().nodeOverrides.at(1).position,{4,0,0}),"Redo restores final drag state");
        const auto copy=DeserializeSceneDocument(SerializeSceneDocument(f.scene.ToDocument()));
        Require(Near(copy.objects.front().nodeOverrides.at(1).position,{4,0,0}),"Gizmo edit survives save/load");
        Require(Near(initial.objects.front().transform.position,f.Object().transform.position),"Node gizmo does not edit owner transform");
        const auto sibling=f.query.WorldBounds(2,f.Object().transform.ModelMatrix(),f.Object().nodeOverrides);
        Require(sibling && Near(sibling->minimum,{118,10,0}),"Sibling geometry stays unchanged");
        Require(Near(glm::vec3{f.query.Evaluate({})[1][3]},{10,1,0}),"Shared asset stays unchanged");
    }
    void HistoryBoundsStayInSync()
    {
        Fixture f;
        const auto refresh=[&]
        {
            RefreshModelNodeBounds(f.scene.Objects(),[&](const std::string&) { return &f.query; });
        };
        refresh();
        desired=f.Base();desired[3]+=glm::vec4{0,40,0,0};changed=true;active=true;f.Frame();f.Release();
        const Renderer::Ray movedRay{{119.5f,42.5f,10},{0,0,-1}};
        const auto hitsMovedLocation=[&]
        {
            return Renderer::IntersectRayTransformedBox(movedRay,*f.Object().localBounds,
                f.Object().transform.ModelMatrix()).has_value();
        };
        Require(hitsMovedLocation(),"Whole-model picking covers moved node before undo");
        Require(f.editor.Undo(f.scene),"Undo node move for bounds regression");
        refresh();
        Require(!hitsMovedLocation(),"Undo removes old picking volume in the same frame");
        const auto afterUndo=CalculateSceneBounds(f.scene.Objects(),*f.Object().localBounds);
        Require(afterUndo.maximum.y<13,"Shadow fitting bounds return to original geometry on undo");
        Require(f.editor.Redo(f.scene),"Redo node move for bounds regression");
        refresh();
        Require(hitsMovedLocation(),"Redo restores moved picking volume in the same frame");
        const auto afterRedo=CalculateSceneBounds(f.scene.Objects(),*f.Object().localBounds);
        Require(afterRedo.maximum.y>43,"Shadow fitting bounds follow the redone geometry");
    }
    void RotationScaleAndGroups()
    {
        Fixture f;Transform rotation;rotation.rotationRadians.z=glm::radians(30.f);
        desired=f.Base()*rotation.GizmoMatrix();changed=true;active=true;f.Frame(GizmoOperation::Rotate,GizmoSpace::Local);
        Require(observedSpace==GizmoSpace::Local,"Gizmo receives local coordinate space");
        f.Release(GizmoOperation::Rotate);
        Require(std::abs(f.Object().nodeOverrides.at(1).rotationRadians.z-rotation.rotationRadians.z)<.0001f,"Rotation becomes local adjustment");
        auto scale=f.Object().nodeOverrides.at(1);scale.scale={2,3,4};
        desired=f.Base()*scale.GizmoMatrix();changed=true;active=true;f.Frame(GizmoOperation::Scale,GizmoSpace::Local);
        f.Release(GizmoOperation::Scale);
        Require(Near(f.Object().nodeOverrides.at(1).scale,{2,3,4}),"Local scale gizmo preserves rotation");
        Require(f.editor.Undo(f.scene),"Undo scale");
        Require(Near(f.Object().nodeOverrides.at(1).scale,{1,1,1}),"Undo scale keeps earlier rotation");
        f.selection.SelectNode(f.id,0,"model.glb");auto base=f.Base(0);
        desired=base;desired[3]+=glm::vec4{10,0,0,0};changed=true;active=true;f.Frame();f.Release();
        const auto root=f.query.WorldBounds(0,f.Object().transform.ModelMatrix(),f.Object().nodeOverrides);
        Require(root && root->minimum.x>125,"Group gizmo moves descendants");
    }
    void GuardsAndPinnedTarget()
    {
        Fixture f;desired=f.Base();desired[3]+=glm::vec4{0,4,0,0};changed=true;active=true;
        f.Frame(GizmoOperation::Translate,GizmoSpace::World,true);
        Require(f.Object().nodeOverrides.empty() && enabledDraws==0,"Navigation disables node manipulation");
        f.Frame(GizmoOperation::Translate,GizmoSpace::World,false,false);
        Require(f.Object().nodeOverrides.empty(),"Drag cannot start outside Scene");
        static_cast<void>(f.scene.SetObjectVisible(f.id,false));f.Frame();
        Require(f.Object().nodeOverrides.empty(),"Hidden owner has no node gizmo");
        static_cast<void>(f.scene.SetObjectVisible(f.id,true));f.Frame();
        f.selection.SelectNode(f.id,2,"model.glb");desired[3]+=glm::vec4{0,4,0,0};f.Frame();f.Release();
        Require(f.Object().nodeOverrides.contains(1) && !f.Object().nodeOverrides.contains(2),"Active drag stays on original target after selection changes");
    }
    void RejectionAndNoOps()
    {
        Fixture f;desired=f.Base();desired[1]+=desired[0]*.5f;changed=true;active=true;f.Frame(GizmoOperation::Rotate);
        Require(f.Object().nodeOverrides.empty() && !f.editor.NodeGizmoMessage().empty(),"Shearing drag is rejected with feedback");
        f.Release();Require(!f.editor.CanUndo(),"Rejected drag records no history");
        desired=f.Base();changed=true;active=true;f.Frame();f.Release();
        Require(f.Object().nodeOverrides.empty() && !f.editor.CanUndo(),"Unchanged drag records no override or history");
        const auto initial=f.Base();desired=initial;desired[3]+=glm::vec4{0,4,0,0};changed=true;active=true;f.Frame();
        desired=initial;f.Frame();f.Release();
        Require(f.Object().nodeOverrides.empty() && !f.editor.CanUndo(),"Returning a drag to its start leaves no override or history");
        auto singular=glm::scale(glm::mat4{1},glm::vec3{0,1,1});Transform value;
        Require(!TryApplyNodeGizmoTransform(value,singular,glm::mat4{1},GizmoOperation::Translate),"Singular parent prefix is rejected");
        auto sheared=glm::mat4{1};sheared[1].x=.5f;
        auto world=sheared;world[3]={3,2,1,1};
        Require(TryApplyNodeGizmoTransform(value,sheared,world,GizmoOperation::Translate) && Near(value.position,{2,2,1}),"Translation preserves authored shear");
        value={};const auto mirrored=glm::scale(glm::mat4{1},glm::vec3{-2,3,4});world=mirrored;world[3]={2,6,12,1};
        Require(TryApplyNodeGizmoTransform(value,mirrored,world,GizmoOperation::Translate) && Near(value.position,{-1,2,3}),"Mirrored parent maps world translation correctly");
        value={};const auto nonuniform=glm::scale(glm::mat4{1},glm::vec3{2,1,1});
        world=glm::rotate(glm::mat4{1},glm::radians(45.f),glm::vec3{0,0,1})*nonuniform;
        Require(!TryApplyNodeGizmoTransform(value,nonuniform,world,GizmoOperation::Rotate) && Near(value.rotationRadians,{0,0,0}),"Unrepresentable world rotation under nonuniform parent preserves adjustment");
        auto tiny=glm::scale(glm::mat4{1},glm::vec3{1e-8f});world=tiny;world[3]={1e-8f,2e-8f,3e-8f,1};value={};
        Require(TryApplyNodeGizmoTransform(value,tiny,world,GizmoOperation::Translate) && Near(value.position,{1,2,3}),"Small-scale parent inverse uses double precision");
        f.asset.nodes[0].localTransform.scale={0,1,1};f.query=ModelNodeQuery{f.asset};enabledDraws=0;changed=true;active=true;f.Frame();
        Require(enabledDraws==0 && !f.editor.NodeGizmoMessage().empty(),"Singular hierarchy disables gizmo with feedback");
    }
}
int main()
{
    try{DragAndHistory();HistoryBoundsStayInSync();RotationScaleAndGroups();GuardsAndPinnedTarget();RejectionAndNoOps();}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"Node gizmo hierarchy, drag history, persistence, guards and rejection checks passed\n";
}
