#include "SceneRenderingValidation.hpp"

#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

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
        const SceneRenderingSettings& settings,
        std::string_view property)
    {
        const auto error = ValidateSceneRendering(settings);

        Require(
            error.has_value(),
            "Invalid rendering settings were accepted.");

        Require(
            error->property == property,
            "Validation identified the wrong property.");
    }

    void TestValidSettings()
    {
        SceneRenderingSettings settings;

        Require(
            !ValidateSceneRendering(settings).has_value(),
            "Default rendering settings were rejected.");

        settings.backgroundColor = { 0.0f, 1.0f, 0.5f };
        settings.environmentIntensity = 0.0f;
        settings.showEnvironmentBackground = true;

        Require(
            !ValidateSceneRendering(settings).has_value(),
            "Valid boundary values were rejected.");

        settings.exposure = 4.0f;
        settings.environmentIntensity = 8.0f;

        Require(
            !ValidateSceneRendering(settings).has_value(),
            "Rendering intensity values above one were rejected.");
    }

    void TestInvalidSettings()
    {
        const std::array invalidNumbers{
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };

        for (const float value : invalidNumbers)
        {
            SceneRenderingSettings settings;
            settings.exposure = value;
            ExpectError(settings, "exposure");

            settings = SceneRenderingSettings{};
            settings.environmentIntensity = value;
            ExpectError(settings, "environmentIntensity");
        }

        for (const float value : { 0.0f, -1.0f })
        {
            SceneRenderingSettings settings;
            settings.exposure = value;
            ExpectError(settings, "exposure");
        }

        {
            SceneRenderingSettings settings;
            settings.environmentIntensity = -0.01f;
            ExpectError(settings, "environmentIntensity");
        }

        const std::array invalidColors{
            -0.01f,
            1.01f,
            invalidNumbers[0],
            invalidNumbers[1],
            invalidNumbers[2]
        };

        for (int component = 0; component < 3; ++component)
        {
            for (const float value : invalidColors)
            {
                SceneRenderingSettings settings;
                settings.backgroundColor[component] = value;
                ExpectError(settings, "backgroundColor");
            }
        }
    }

    void TestShadowSettings()
    {
        SceneRenderingSettings settings;
        settings.directionalShadows.enabled = false;
        settings.directionalShadows.filteringEnabled = false;
        settings.directionalShadows.minimumBias = 0.0f;
        settings.directionalShadows.slopeBias = 0.0f;

        Require(
            !ValidateSceneRendering(settings),
            "Zero shadow biases were rejected.");

        settings.directionalShadows.minimumBias = 0.01f;
        settings.directionalShadows.slopeBias = 0.02f;

        Require(
            !ValidateSceneRendering(settings),
            "Maximum supported shadow biases were rejected.");

        for (const float value : {
            -0.001f,
                0.011f,
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity() })
        {
            settings = SceneRenderingSettings{};
            settings.directionalShadows.enabled = false;
            settings.directionalShadows.minimumBias = value;

            ExpectError(
                settings,
                "directionalShadows.minimumBias");
        }

        for (const float value : {
            -0.001f,
                0.021f,
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity() })
        {
            settings = SceneRenderingSettings{};
            settings.directionalShadows.slopeBias = value;

            ExpectError(
                settings,
                "directionalShadows.slopeBias");
        }
    }
}

int main()
{
    try
    {
        TestValidSettings();
        TestInvalidSettings();
        TestShadowSettings();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}