#include "SelectionOrbitController.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <glm/trigonometric.hpp>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
    bool Near(glm::vec3 a,glm::vec3 b) { return glm::length(a-b)<.0001f; }
    const Renderer::AxisAlignedBoundingBox bounds{{10,0,-1},{12,4,1}};
    const Renderer::AxisAlignedBoundingBox world{{-100,-50,-100},{100,100,100}};
    const glm::vec3 center{11,2,0};
    Renderer::PerspectiveCamera Original() { return {{20,10,30},center,{0,1,0},glm::radians(60.0f),.05f,1000}; }
    void RequireCentered(const Renderer::PerspectiveCamera& camera,glm::vec3 pivot)
    {
        Require(glm::dot(camera.ForwardDirection(),glm::normalize(pivot-camera.Position()))>.99999f,"Orbit stays aimed at bounds center");
        Require(std::isfinite(camera.ViewMatrix()[0][0]) && camera.NearClippingPlane()>0 &&
            camera.FarClippingPlane()>camera.NearClippingPlane(),"Orbit produces a finite valid camera");
    }
    void TestMovement()
    {
        SelectionOrbitController orbit;SceneSelection selection;selection.SelectObject(1);
        Require(!orbit.Active(),"Orbit starts inactive");
        const auto original=Original();
        auto camera=orbit.Start(selection,bounds,world,original,1.6f);
        Require(orbit.Active(),"Start enables orbit");RequireCentered(camera,center);
        const float radius=glm::length(camera.Position()-center);
        const auto initial=camera;
        camera=orbit.Update(bounds,world,camera,80,20,0);
        RequireCentered(camera,center);
        Require(!Near(camera.Position(),initial.Position()) && std::abs(glm::length(camera.Position()-center)-radius)<.0001f,
            "Mouse orbit changes angle while retaining radius");
        const auto rotated=camera;
        camera=orbit.Update(bounds,world,camera,0,0,3);
        Require(glm::length(camera.Position()-center)<radius && Near(camera.ForwardDirection(),rotated.ForwardDirection()),
            "Wheel dollies toward pivot without changing viewing direction");
        camera=orbit.Update(bounds,world,camera,0,0,-3);
        Require(Near(camera.Position(),rotated.Position()),"Opposite wheel movement restores radius");
        Require(camera.VerticalFieldOfViewRadians()==original.VerticalFieldOfViewRadians(),"Orbit zoom preserves FOV");
        camera=orbit.Update(bounds,world,camera,100000,100000,0);
        RequireCentered(camera,center);
        Require(std::abs(glm::dot(camera.ForwardDirection(),glm::vec3{0,1,0}))<1,"Pitch clamps before pole singularity");
        camera=orbit.Update(bounds,world,camera,0,0,128);RequireCentered(camera,center);
        Require(glm::length(camera.Position()-center)>camera.NearClippingPlane(),"Zoom cannot pass through pivot or near clip");
        camera=orbit.Update(bounds,world,camera,0,0,-128);RequireCentered(camera,center);
        orbit.Stop();
        const auto stopped=orbit.Update(bounds,world,camera,100,100,10);
        Require(!orbit.Active() && Near(stopped.Position(),camera.Position()) && Near(stopped.ForwardDirection(),camera.ForwardDirection()),
            "Exit leaves current pose unchanged");
    }
    void TestSelectionAndTracking()
    {
        SelectionOrbitController orbit;SceneSelection selection;selection.AddObject(1);selection.AddObject(2);
        auto camera=orbit.Start(selection,bounds,world,Original(),1.6f);
        selection.Clear();selection.AddObject(2);selection.AddObject(1);orbit.ObserveSelection(selection);
        Require(orbit.Active(),"Reordered object group retains orbit target");
        const Renderer::AxisAlignedBoundingBox moved{bounds.minimum+glm::vec3{3,4,5},bounds.maximum+glm::vec3{3,4,5}};
        const auto tracked=orbit.Update(moved,world,camera,0,0,0);
        Require(Near(tracked.Position()-camera.Position(),{3,4,5}),"Moved target tracks pivot without changing orbit offset");
        RequireCentered(tracked,center+glm::vec3{3,4,5});
        selection.RemoveObject(1);orbit.ObserveSelection(selection);
        Require(!orbit.Active(),"Changing group membership exits orbit");
        selection.SelectNode(1,3,"room.glb");camera=orbit.Start(selection,bounds,world,Original(),1.6f);
        selection.SelectNode(2,3,"room.glb");orbit.ObserveSelection(selection);
        Require(!orbit.Active(),"Same node in another instance exits orbit");
        selection.SelectLight(1);camera=orbit.Start(selection,bounds,world,Original(),1.6f);
        RequireCentered(camera,center);selection.SelectLight(2);orbit.ObserveSelection(selection);
        Require(!orbit.Active(),"Changing light exits orbit");
        selection.SelectObject(1);camera=orbit.Start(selection,bounds,world,Original(),1.6f);
        selection.Clear();orbit.ObserveSelection(selection);Require(!orbit.Active(),"Deselecting exits orbit");
    }
    void TestInvalidInputAndClipping()
    {
        SelectionOrbitController orbit;SceneSelection selection;selection.SelectObject(1);
        const auto initial=orbit.Start(selection,bounds,world,Original(),1.6f);
        const auto ignored=orbit.Update(bounds,world,initial,std::numeric_limits<double>::infinity(),0,0);
        Require(Near(initial.Position(),ignored.Position()),"Ignore nonfinite input");
        auto invalid=bounds;invalid.minimum.x=std::numeric_limits<float>::quiet_NaN();
        Require(Near(orbit.Update(invalid,world,initial,0,0,1).Position(),initial.Position()),"Ignore invalid target bounds");
        const auto far=orbit.Update(bounds,world,initial,40,-10,-20);
        for(unsigned int corner=0;corner<8;++corner)
        {
            const glm::vec3 p{corner&1u?world.maximum.x:world.minimum.x,corner&2u?world.maximum.y:world.minimum.y,
                corner&4u?world.maximum.z:world.minimum.z};
            Require(glm::length(p-far.Position())<far.FarClippingPlane(),"Far clipping covers scene at changed orbit positions");
        }
        orbit.Stop();selection.Clear();bool rejected=false;
        try { static_cast<void>(orbit.Start(selection,bounds,world,Original(),1.6f)); }
        catch(const std::invalid_argument&) { rejected=true; }
        Require(rejected && !orbit.Active(),"Empty selection cannot activate orbit");
    }
}
int main()
{
    try { TestMovement();TestSelectionAndTracking();TestInvalidInputAndClipping(); }
    catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
    std::cout<<"Selection orbit rotation, zoom, target identity, clipping and exit tests passed\n";
}
