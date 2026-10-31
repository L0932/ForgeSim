#include "SceneCameraData.hpp"
#include "SceneCameraValidation.hpp"

#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

#include <glm/geometric.hpp>

namespace
{
    using namespace ForgeSim::Sandbox;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void ExpectError(
        const SceneCameraData& camera,
        std::string_view property)
    {
        const auto error = ValidateSceneCamera(camera);

        Require(error.has_value(), "Invalid camera was accepted.");
        Require(
            error->property == property,
            "Validation reported the wrong property.");
        Require(
            !error->message.empty(),
            "Validation must explain the error.");
    }

    void TestValidCameras()
    {
        SceneCameraData camera;

        Require(
            !ValidateSceneCamera(camera),
            "Default camera must be valid.");

        // A rolled camera is valid; up need not be world Y.
        camera.upDirection = { 1.0f, 0.0f, 0.0f };

        Require(
            !ValidateSceneCamera(camera),
            "Rolled camera must be valid.");

        // Small scenes must remain supported.
        camera.nearClippingPlane = 0.00001f;
        camera.farClippingPlane = 0.01f;

        Require(
            !ValidateSceneCamera(camera),
            "Small positive clipping distances must be valid.");

        // Permit ordinary rounding without normalizing stored data.
        camera.forwardDirection = { 0.0f, 0.0f, -1.0001f };
        const SceneCameraData before = camera;

        Require(
            !ValidateSceneCamera(camera),
            "Small direction rounding error must be accepted.");

        Require(
            camera.position == before.position &&
            camera.forwardDirection == before.forwardDirection &&
            camera.upDirection == before.upDirection &&
            camera.verticalFieldOfViewRadians ==
            before.verticalFieldOfViewRadians &&
            camera.nearClippingPlane == before.nearClippingPlane &&
            camera.farClippingPlane == before.farClippingPlane,
            "Validation must not modify camera data.");
    }

    void TestNonFiniteValues()
    {
        for (const float value : {
            std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity()
        })
        {
            SceneCameraData camera;
            camera.position.x = value;
            ExpectError(camera, "position");

            camera = SceneCameraData{};
            camera.forwardDirection.y = value;
            ExpectError(camera, "forwardDirection");

            camera = SceneCameraData{};
            camera.upDirection.z = value;
            ExpectError(camera, "upDirection");

            camera = SceneCameraData{};
            camera.verticalFieldOfViewRadians = value;
            ExpectError(camera, "verticalFieldOfViewRadians");

            camera = SceneCameraData{};
            camera.nearClippingPlane = value;
            ExpectError(camera, "nearClippingPlane");

            camera = SceneCameraData{};
            camera.farClippingPlane = value;
            ExpectError(camera, "farClippingPlane");
        }
    }

    void TestDirections()
    {
        for (const glm::vec3 direction : {
            glm::vec3{ 0.0f },
                glm::vec3{ 0.0f, 0.0f, -2.0f },
                glm::vec3{ std::numeric_limits<float>::max() }
        })
        {
            SceneCameraData camera;
            camera.forwardDirection = direction;
            ExpectError(camera, "forwardDirection");

            camera = SceneCameraData{};
            camera.upDirection = direction;
            ExpectError(camera, "upDirection");
        }

        // Parallel and antiparallel directions cannot form a basis.
        for (const float sign : { -1.0f, 1.0f })
        {
            SceneCameraData camera;
            camera.upDirection = camera.forwardDirection * sign;
            ExpectError(camera, "upDirection");
        }

        SceneCameraData camera;
        camera.upDirection =
            glm::normalize(glm::vec3{ 0.0f, 0.0001f, -1.0f });

        ExpectError(camera, "upDirection");
    }

    void TestProjectionLimits()
    {
        for (const float fieldOfView : {
            -0.1f,
                0.0f,
                glm::radians(180.0f),
                glm::radians(181.0f)
        })
        {
            SceneCameraData camera;
            camera.verticalFieldOfViewRadians = fieldOfView;
            ExpectError(camera, "verticalFieldOfViewRadians");
        }

        for (const float nearDistance : { -1.0f, 0.0f })
        {
            SceneCameraData camera;
            camera.nearClippingPlane = nearDistance;
            ExpectError(camera, "nearClippingPlane");
        }

        SceneCameraData camera;

        for (const float farDistance : {
            -1.0f,
                0.0f,
                camera.nearClippingPlane * 0.5f,
                camera.nearClippingPlane
        })
        {
            camera.farClippingPlane = farDistance;
            ExpectError(camera, "farClippingPlane");
        }
    }
}

int main()
{
    try
    {
        TestValidCameras();
        TestNonFiniteValues();
        TestDirections();
        TestProjectionLimits();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}