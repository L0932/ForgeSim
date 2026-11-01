#include "SceneCameraState.hpp"
#include "SceneCameraValidation.hpp"

#include <stdexcept>
#include <string>

#include <glm/geometric.hpp>

namespace ForgeSim::Sandbox
{
    Renderer::PerspectiveCamera CreateSceneCamera(const SceneCameraData& camera)
    {
        if (const auto error = ValidateSceneCamera(camera))
        {
            throw std::invalid_argument("camera." + std::string{ error->property } +
                ": " + std::string{ error->message });
        }
        return Renderer::PerspectiveCamera{
            camera.position, camera.position + glm::normalize(camera.forwardDirection),
            camera.upDirection, camera.verticalFieldOfViewRadians,
            camera.nearClippingPlane, camera.farClippingPlane };
    }

    SceneCameraData CaptureSceneCamera(
        const Renderer::PerspectiveCamera& camera,
        std::optional<SceneCaptureExtent> referenceViewport)
    {
        SceneCameraData result{
            .position = camera.Position(),
            .forwardDirection = camera.ForwardDirection(),
            .upDirection = camera.UpDirection(),
            .verticalFieldOfViewRadians = camera.VerticalFieldOfViewRadians(),
            .nearClippingPlane = camera.NearClippingPlane(),
            .farClippingPlane = camera.FarClippingPlane(),
            .referenceViewport = referenceViewport
        };
        if (const auto error = ValidateSceneCamera(result))
        {
            throw std::invalid_argument("camera." + std::string{ error->property } +
                ": " + std::string{ error->message });
        }
        return result;
    }
}
