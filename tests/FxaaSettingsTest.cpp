#include <ForgeSim/Renderer/FxaaSettings.hpp>

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace
{
    using ForgeSim::Renderer::CalculateFxaaTexelSize;
    using ForgeSim::Renderer::FxaaSettings;
    using ForgeSim::Renderer::ValidateFxaaSettings;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void TestSettingsValidation()
    {
        ValidateFxaaSettings(FxaaSettings{});

        constexpr std::array fields{
            &FxaaSettings::relativeContrastThreshold,
            &FxaaSettings::minimumContrastThreshold,
            &FxaaSettings::subpixelBlending
        };
        constexpr std::array fieldNames{
            std::string_view{"relative contrast threshold"},
            std::string_view{"minimum contrast threshold"},
            std::string_view{"subpixel blending"}
        };
        const std::array invalidValues{
            -0.01f, 1.01f,
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };

        for (std::size_t index = 0; index < fields.size(); ++index)
        {
            for (bool enabled : {false, true})
            {
                for (float boundary : {0.0f, 1.0f})
                {
                    FxaaSettings settings;
                    settings.enabled = enabled;
                    settings.*fields[index] = boundary;
                    ValidateFxaaSettings(settings);
                }

                for (float value : invalidValues)
                {
                    FxaaSettings settings;
                    settings.enabled = enabled;
                    settings.*fields[index] = value;
                    bool rejected = false;
                    try
                    {
                        ValidateFxaaSettings(settings);
                    }
                    catch (const std::invalid_argument& error)
                    {
                        rejected = true;
                        Require(std::string_view{error.what()}.find(
                            fieldNames[index]) != std::string_view::npos,
                            "Diagnostic must identify the invalid control.");
                    }
                    Require(rejected,
                        "Invalid controls must be rejected even when FXAA is disabled.");
                }
            }
        }
    }

    void TestSourceDimensions()
    {
        Require(!CalculateFxaaTexelSize(0, 0) &&
            !CalculateFxaaTexelSize(0, 1080) &&
            !CalculateFxaaTexelSize(1920, 0),
            "Minimized or empty sources must request a skipped pass.");

        const auto pixel = CalculateFxaaTexelSize(1, 1);
        Require(pixel && (*pixel)[0] == 1.0f && (*pixel)[1] == 1.0f,
            "A one-pixel source must remain valid.");

        const auto landscape = CalculateFxaaTexelSize(1920, 1080);
        const auto portrait = CalculateFxaaTexelSize(1080, 1920);
        Require(landscape && portrait,
            "Both viewport orientations must be supported.");
        Require(std::abs((*landscape)[0] * 1920.0f - 1.0f) < 0.000001f &&
            std::abs((*landscape)[1] * 1080.0f - 1.0f) < 0.000001f,
            "UV offsets must span exactly one source pixel on each axis.");
        Require((*landscape)[0] == (*portrait)[1] &&
            (*landscape)[1] == (*portrait)[0],
            "Resizing must update each source axis independently.");

        const auto maximum = CalculateFxaaTexelSize(
            std::numeric_limits<std::uint32_t>::max(), 1);
        Require(maximum && std::isfinite((*maximum)[0]) &&
            (*maximum)[0] > 0.0f && (*maximum)[1] == 1.0f,
            "CPU dimension arithmetic must not overflow or divide by zero.");
    }
}

int main()
{
    try
    {
        TestSettingsValidation();
        TestSourceDimensions();
        std::cout << "FXAA settings tests passed.\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
