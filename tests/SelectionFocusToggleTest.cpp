#include "SelectionFocusToggle.hpp"
#include <iostream>
#include <stdexcept>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
    bool Near(glm::vec3 a, glm::vec3 b) { return glm::length(a - b) < 0.0001f; }
    bool SameView(const Renderer::PerspectiveCamera& a, const Renderer::PerspectiveCamera& b)
    {
        return Near(a.Position(),b.Position()) && Near(a.ForwardDirection(),b.ForwardDirection()) &&
            Near(a.UpDirection(),b.UpDirection()) && std::abs(a.VerticalFieldOfViewRadians()-b.VerticalFieldOfViewRadians()) < .00001f &&
            a.NearClippingPlane()==b.NearClippingPlane() && a.FarClippingPlane()==b.FarClippingPlane();
    }
    Renderer::PerspectiveCamera Original()
    { return {{20,10,30},{0,0,0},{0,1,0},glm::radians(60.0f),.05f,2000}; }
    const Renderer::AxisAlignedBoundingBox bounds{{-1,-2,-1},{1,2,1}};
    const Renderer::AxisAlignedBoundingBox world{{-100,-10,-100},{100,30,100}};
    void TestRoundTrips()
    {
        SceneSelection selection; selection.SelectObject(1);
        SelectionFocusToggle toggle;
        const auto original=Original();
        const auto first=toggle.Toggle(selection,bounds,world,original,1.6f);
        Require(!Near(first.Position(),original.Position()), "First request frames selection");
        Require(Near(first.ForwardDirection(),original.ForwardDirection()) && first.VerticalFieldOfViewRadians()==original.VerticalFieldOfViewRadians(),
            "Focus preserves viewing angle and FOV");
        const auto back=toggle.Toggle(selection,bounds,world,CreateSceneCamera(CaptureSceneCamera(first)),1.6f);
        Require(SameView(back,original), "Second request restores full previous view, including clipping");
        const auto third=toggle.Toggle(selection,bounds,world,back,1.6f);
        Require(SameView(third,first), "Third request focuses again");
        const auto fourth=toggle.Toggle(selection,bounds,world,third,1.6f);
        Require(SameView(fourth,original), "Repeated F and double-click requests share the same toggle");
    }
    void TestSelectionIdentity()
    {
        const auto original=Original();
        SelectionFocusToggle toggle;
        SceneSelection selection; selection.SelectObject(1);
        const auto first=toggle.Toggle(selection,bounds,world,original,1.6f);
        selection.SelectObject(2);
        const auto second=toggle.Toggle(selection,bounds,world,first,1.6f);
        const auto restored=toggle.Toggle(selection,bounds,world,second,1.6f);
        Require(SameView(restored,first), "Different object starts a fresh return-view pair");
        toggle.Reset();
        selection.SelectNode(1,3,"room.glb");
        const auto node=toggle.Toggle(selection,bounds,world,original,1.6f);
        selection.SelectNode(2,3,"room.glb");
        const auto otherInstance=toggle.Toggle(selection,bounds,world,node,1.6f);
        Require(SameView(toggle.Toggle(selection,bounds,world,otherInstance,1.6f),node), "Same node in another instance has distinct focus identity");
        toggle.Reset();
        selection.SelectLight(1);
        const auto light=toggle.Toggle(selection,bounds,world,original,1.6f);
        Require(SameView(toggle.Toggle(selection,bounds,world,light,1.6f),original), "Lights support round trip");
        selection.Clear(); selection.AddObject(1); selection.AddObject(2);
        const auto group=toggle.Toggle(selection,bounds,world,original,1.6f);
        selection.Clear(); selection.AddObject(2); selection.AddObject(1);
        Require(SameView(toggle.Toggle(selection,bounds,world,group,1.6f),original), "Group identity ignores active-member ordering");
        const auto focused=toggle.Toggle(selection,bounds,world,original,1.6f);
        selection.Clear(); toggle.ObserveSelection(selection);
        selection.AddObject(1); selection.AddObject(2); toggle.ObserveSelection(selection);
        const auto restarted=toggle.Toggle(selection,bounds,world,focused,1.6f);
        Require(!SameView(restarted,original), "Deselecting and reselecting cannot restore a stale view");
    }
    void TestNavigationAndSceneChanges()
    {
        const auto original=Original();
        SceneSelection selection; selection.SelectObject(1);
        SelectionFocusToggle toggle;
        auto focused=toggle.Toggle(selection,bounds,world,original,1.6f);
        focused.Translate({5,2,1});
        const auto navigated=focused;
        const auto reframed=toggle.Toggle(selection,bounds,world,navigated,1.6f);
        Require(SameView(toggle.Toggle(selection,bounds,world,reframed,1.6f),navigated), "Manual navigation establishes a new return view");
        auto rotated=toggle.Toggle(selection,bounds,world,navigated,1.6f);
        rotated.Rotate(.2f,.1f);
        const auto refocus=toggle.Toggle(selection,bounds,world,rotated,1.6f);
        Require(SameView(toggle.Toggle(selection,bounds,world,refocus,1.6f),rotated), "Manual rotation establishes a new return view");
        const auto prior=toggle.Toggle(selection,bounds,world,original,1.6f);
        const Renderer::AxisAlignedBoundingBox moved{{20,0,0},{22,4,2}};
        const auto movedFocus=toggle.Toggle(selection,moved,world,prior,1.6f);
        Require(SameView(toggle.Toggle(selection,moved,world,movedFocus,1.6f),prior), "Moved selected geometry reframes instead of restoring stale view");
        const auto beforeReset=toggle.Toggle(selection,bounds,world,original,1.6f);
        toggle.Reset(); // New/Open/import or explicit Frame scene.
        const auto afterReset=toggle.Toggle(selection,bounds,world,beforeReset,1.6f);
        Require(!SameView(afterReset,original), "Scene reset invalidates return camera even when IDs are reused");
    }
}
int main()
{
    try { TestRoundTrips(); TestSelectionIdentity(); TestNavigationAndSceneChanges(); }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    std::cout << "Selection focus toggle round trips, selection identity and reset behavior passed\n";
}
