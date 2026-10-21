#include "SceneBounds.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <numbers>
#include <stdexcept>

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

    [[nodiscard]] bool Near(
        const glm::vec3& actual,
        const glm::vec3& expected)
    {
        constexpr float tolerance = 1e-4f;

        return std::abs(actual.x - expected.x) < tolerance &&
            std::abs(actual.y - expected.y) < tolerance &&
            std::abs(actual.z - expected.z) < tolerance;
    }

    void TestTransformedBounds()
    {
        const ForgeSim::Renderer::AxisAlignedBoundingBox local{
            .minimum = { -1.0f, -2.0f, -3.0f },
            .maximum = { 1.0f, 2.0f, 3.0f }
        };

        std::array<SandboxObject, 1> objects{};

        objects[0].transform.position = { 10.0f, 0.0f, 0.0f };
        objects[0].transform.rotationRadians.z =
            std::numbers::pi_v<float> *0.5f;
        objects[0].transform.scale = { -2.0f, 1.0f, 0.5f };

        const auto bounds = CalculateSceneBounds(objects, local);

        Require(
            Near(bounds.minimum, { 8.0f, -2.0f, -1.5f }) &&
            Near(bounds.maximum, { 12.0f, 2.0f, 1.5f }),
            "Translation, rotation, or signed scale bounds are wrong.");
    }

    void TestMultipleObjects()
    {
        const ForgeSim::Renderer::AxisAlignedBoundingBox local{
            .minimum = { -1.0f, -1.0f, -1.0f },
            .maximum = { 1.0f, 1.0f, 1.0f }
        };

        std::array<SandboxObject, 2> objects{};

        objects[0].transform.position.x = -10.0f;
        objects[1].transform.position.x = 20.0f;

        const auto bounds = CalculateSceneBounds(objects, local);

        Require(
            Near(bounds.minimum, { -11.0f, -1.0f, -1.0f }) &&
            Near(bounds.maximum, { 21.0f, 1.0f, 1.0f }),
            "Scene bounds did not include every object.");
    }

    void TestEmptyScene()
    {
        const ForgeSim::Renderer::AxisAlignedBoundingBox local{
            .minimum = { -1.0f, -2.0f, -3.0f },
            .maximum = { 1.0f, 2.0f, 3.0f }
        };

        const auto bounds = CalculateSceneBounds({}, local);

        Require(
            Near(bounds.minimum, local.minimum) &&
            Near(bounds.maximum, local.maximum),
            "Empty-scene bounds did not use the fallback.");
    }
}

int main()
{
    try
    {
        TestTransformedBounds();
        TestMultipleObjects();
        TestEmptyScene();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}