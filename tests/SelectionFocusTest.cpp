#include "SelectionFocus.hpp"
#include "SceneCameraFraming.hpp"
#include "EditorShortcutContext.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <glm/vec4.hpp>

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
        const Renderer::AxisAlignedBoundingBox ground{{-40,-0.001f,-10}, {40,0.001f,10}};
        const Renderer::AxisAlignedBoundingBox fullScene{{-40,-2,-10}, {500,80,90}};
        const Renderer::PerspectiveCamera original{{0,30,35}, {0,0,0}, {0,1,0}, 0.9f, 0.05f, 10.0f};
        for (float aspect : {0.5f, 2.4f})
        {
            const auto focusedCamera = CreateSelectionFocusCamera(ground, fullScene, original, aspect);
            Require(Near(focusedCamera.ForwardDirection(), original.ForwardDirection()),
                "Focus must preserve the viewing direction for flat models");
            Require(focusedCamera.VerticalFieldOfViewRadians() == original.VerticalFieldOfViewRadians(),
                "Focus must preserve the user's field of view");
            const auto projectionView = focusedCamera.ProjectionMatrix(aspect) * focusedCamera.ViewMatrix();
            for (unsigned int corner = 0; corner < 8; ++corner)
            {
                const glm::vec3 point{(corner & 1u) ? ground.maximum.x : ground.minimum.x,
                    (corner & 2u) ? ground.maximum.y : ground.minimum.y,
                    (corner & 4u) ? ground.maximum.z : ground.minimum.z};
                const auto clip = projectionView * glm::vec4{point, 1.0f};
                Require(clip.w > 0 && std::abs(clip.x / clip.w) < 1 && std::abs(clip.y / clip.w) < 1 &&
                    clip.z / clip.w > -1 && clip.z / clip.w < 1,
                    "Focus must fit every selection corner in wide and portrait viewports");
            }
        }
        const auto fittedGround = CreateSelectionFocusCamera(ground, fullScene, original, 2.4f);
        const auto oldGround = CreateSceneCameraFraming(ground, 2.4f).camera;
        Require(glm::length(fittedGround.Position()) < glm::length(oldGround.Position()) * 0.8f,
            "Long flat models should frame more tightly than the old bounding sphere");
        const Renderer::AxisAlignedBoundingBox lightBounds{{-0.1f,2,-0.1f}, {0.1f,2.2f,0.1f}};
        const auto lightCamera = CreateSelectionFocusCamera(lightBounds, fullScene, original, 2.4f);
        for (unsigned int corner = 0; corner < 8; ++corner)
        {
            const glm::vec3 point{(corner & 1u) ? fullScene.maximum.x : fullScene.minimum.x,
                (corner & 2u) ? fullScene.maximum.y : fullScene.minimum.y,
                (corner & 4u) ? fullScene.maximum.z : fullScene.minimum.z};
            Require(glm::length(point - lightCamera.Position()) < lightCamera.FarClippingPlane(),
                "Focusing a tiny light must retain far clipping for the whole scene in every direction");
        }
        Require(lightCamera.NearClippingPlane() <= original.NearClippingPlane() &&
            lightCamera.FarClippingPlane() >= original.FarClippingPlane(),
            "Selection focus must not narrow the existing clipping range");
        std::cout << "Selection bounds, light focus, stale selections, and shortcut contexts passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
