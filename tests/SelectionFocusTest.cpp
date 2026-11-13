#include "SelectionFocus.hpp"
#include "SceneCameraFraming.hpp"
#include "EditorShortcutContext.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace ForgeSim;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool value, const char* message)
    {
        if (!value) throw std::runtime_error(message);
    }
    bool Near(glm::vec3 left, glm::vec3 right) { return glm::length(left - right) < 0.0001f; }
}
int main()
{
    try
    {
        const Renderer::AxisAlignedBoundingBox fallback{{-1,-1,-1}, {1,1,1}};
        std::array<SandboxObject, 2> objects{};
        objects[0].id = 1;
        objects[0].localBounds = Renderer::AxisAlignedBoundingBox{{2,0,0}, {6,2,4}};
        objects[0].transform.pivot = {2,0,0};
        objects[0].transform.position = {10,3,5};
        objects[0].transform.scale = {2,3,4};
        objects[1].id = 2;
        objects[1].transform.position = {1000,0,0};
        DirectionalLight directional{.id = 1, .position = {8,9,10}};
        std::array<PointLight, 1> points{{{.id = 2, .enabled = false, .position = {-4,2,5}, .range = 20}}};
        std::array<SpotLight, 1> spots{{{.id = 3, .position = {1,2,3}, .range = 0.001f}}};
        const SceneLightingView lights{directional, points, spots};
        SceneSelection selection;
        Require(!SelectionFocusBounds(objects, lights, selection, fallback), "Empty selection must not focus scene");
        selection.SelectObject(1);
        const auto focused = SelectionFocusBounds(objects, lights, selection, fallback);
        Require(focused && Near(focused->minimum, {10,3,5}) && Near(focused->maximum, {18,9,21}),
            "Object focus must use selected model bounds and its transform, excluding other instances");
        selection.SelectObject(99);
        Require(!SelectionFocusBounds(objects, lights, selection, fallback), "Stale object selection must be a no-op");
        for (SceneLightId id : {1u,2u,3u})
        {
            selection.SelectLight(id);
            const auto bounds = SelectionFocusBounds(objects, lights, selection, fallback);
            const auto position = id == 1 ? directional.position : id == 2 ? points[0].position : spots[0].position;
            Require(bounds && Near((bounds->minimum + bounds->maximum) * 0.5f, position),
                "Each light type must frame its editor position, including disabled lights");
            const auto camera = CreateSceneCameraFraming(*bounds, 1.6f).camera;
            Require(std::isfinite(camera.Position().x) && camera.NearClippingPlane() > 0,
                "Light focus must produce a finite camera with usable clipping");
        }
        selection.SelectLight(99);
        Require(!SelectionFocusBounds(objects, lights, selection, fallback), "Stale light selection must be a no-op");
        selection.SelectLight(1);
        Require(SelectionFocusBounds({}, lights, selection, fallback).has_value(), "Lights must focus in an empty scene");
        Require(!AllowSceneShortcut({}), "Unfocused panels must not trigger scene shortcuts");
        Require(AllowSceneShortcut({.windowFocused = true}), "Focused scene panels should accept shortcuts");
        for (int blocked = 0; blocked < 5; ++blocked)
        {
            EditorShortcutContext context{.windowFocused = true};
            if (blocked == 0) context.editingBlocked = true;
            if (blocked == 1) context.textInput = true;
            if (blocked == 2) context.itemActive = true;
            if (blocked == 3) context.popupOpen = true;
            if (blocked == 4) context.modifierHeld = true;
            Require(!AllowSceneShortcut(context), "Editing, typing, popups, and modifiers must suppress scene shortcuts");
        }
        std::cout << "Selection bounds, light focus, stale selections, and shortcut contexts passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
