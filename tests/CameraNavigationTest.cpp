#include "SceneCameraFraming.hpp"
#include <ForgeSim/Renderer/DebugGeometry.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace ForgeSim;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
}
int main()
{
    try
    {
        const Renderer::AxisAlignedBoundingBox city{glm::vec3{-1000.0f}, glm::vec3{1000.0f}};
        const Renderer::AxisAlignedBoundingBox prop{glm::vec3{-0.01f}, glm::vec3{0.01f}};
        auto large = Sandbox::CreateSceneCameraFraming(city, 1.6f);
        auto small = Sandbox::CreateSceneCameraFraming(prop, 1.6f);
        Require(large.camera.NearClippingPlane() <= 0.05f, "Large-scene near clip remains suitable for close navigation");
        Require(small.camera.NearClippingPlane() < 0.001f, "Small scene has a proportionally smaller near clip");
        Require(large.controller.MovementSpeed() == 10.0f && small.controller.MovementSpeed() == 10.0f &&
            Sandbox::FreeCameraController{}.MovementSpeed() == 10.0f,
            "Default speed must be 10 for every model scale and a fresh controller");
        Require(Sandbox::CreateSceneCameraFraming(city, 0.5f).controller.MovementSpeed() ==
            large.controller.MovementSpeed(), "Viewport aspect does not change navigation speed");
        large.controller.SetMovementSpeed(0.25f);
        const Renderer::AxisAlignedBoundingBox defaultScene{glm::vec3{-1.0f}, glm::vec3{1.0f}};
        auto navigation = Sandbox::CreateSceneCameraController(prop, 0.001f);
        navigation = Sandbox::CreateSceneCameraController(defaultScene);
        Require(navigation.MovementSpeed() ==
            Sandbox::CreateSceneCameraFraming(defaultScene, 1.6f).controller.MovementSpeed(),
            "Opening a scene without a saved speed must discard the previous scene's manual speed");
        navigation = Sandbox::CreateSceneCameraController(city, 2.5f);
        Require(navigation.MovementSpeed() == 2.5f, "Opening a scene must restore its own saved speed");
        navigation = Sandbox::CreateSceneCameraController(prop);
        Require(navigation.MovementSpeed() == small.controller.MovementSpeed(),
            "Returning to a scene without a saved speed must restore the default 10");
        auto workspace = Sandbox::CreateEmptySceneCameraFraming(1.6f);
        Require(workspace.camera.NearClippingPlane() == 0.05f && workspace.camera.FarClippingPlane() == 1000.0f &&
            workspace.controller.MovementSpeed() == 10.0f,
            "Fresh scenes need workspace clipping and default navigation speed");
        // Simulate replacing the previous scene's camera, including custom clips.
        workspace.camera = Renderer::PerspectiveCamera{{0,0,5}, {0,0,0}, {0,1,0}, 0.9f, 2.0f, 3.0f};
        workspace = Sandbox::CreateEmptySceneCameraFraming(0.5f);
        Require(workspace.camera.NearClippingPlane() == 0.05f && workspace.camera.FarClippingPlane() == 1000.0f,
            "New workspace must not inherit previous near/far values or depend on viewport aspect");
        const Renderer::ReferenceGridSpecification grid;
        const float halfExtent = static_cast<float>(grid.halfLineCount) * grid.spacing;
        for (float x : {-halfExtent, halfExtent})
            for (float z : {-halfExtent, halfExtent})
                Require(glm::length(glm::vec3{x,grid.height,z} - workspace.camera.Position()) <
                    workspace.camera.FarClippingPlane(),
                    "The complete reference grid must remain inside far clipping when rotating");
        Platform::InputState input;
        input.SetKeyState(Platform::Key::W, true);
        auto normal = large.camera;
        auto slow = large.camera;
        large.controller.Update(normal, input, 0.1f);
        input.SetKeyState(Platform::Key::LeftControl, true);
        large.controller.Update(slow, input, 0.1f);
        const float normalDistance = glm::length(normal.Position() - large.camera.Position());
        const float slowDistance = glm::length(slow.Position() - large.camera.Position());
        // Large initial coordinates quantize tiny displacements in float space.
        Require(normalDistance > 0.0f && slowDistance > 0.0f && slowDistance < normalDistance * 0.2f,
            "Ctrl provides substantially slower precision movement");
        Require(large.controller.MovementSpeed() == 0.25f, "Slow modifier does not alter base speed");
        for (float invalid : {0.0f, -1.0f, std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN(), 10001.0f})
        {
            bool failed = false;
            try { large.controller.SetMovementSpeed(invalid); }
            catch (const std::invalid_argument&) { failed = true; }
            Require(failed && large.controller.MovementSpeed() == 0.25f,
                "Invalid manual speed is rejected without changing the setting");
        }
        std::cout << "Scale-aware framing, manual speed, and precision navigation passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
