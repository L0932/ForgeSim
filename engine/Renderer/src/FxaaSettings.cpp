#include <ForgeSim/Renderer/FxaaSettings.hpp>

#include <cmath>
#include <stdexcept>

namespace ForgeSim::Renderer
{
    namespace
    {
        void ValidateUnitInterval(float value, const char* message)
        {
            if (!std::isfinite(value) || value < 0.0f || value > 1.0f)
            {
                throw std::invalid_argument(message);
            }
        }
    }

    void ValidateFxaaSettings(const FxaaSettings& settings)
    {
        ValidateUnitInterval(settings.relativeContrastThreshold,
            "FXAA relative contrast threshold must be finite and in [0, 1].");
        ValidateUnitInterval(settings.minimumContrastThreshold,
            "FXAA minimum contrast threshold must be finite and in [0, 1].");
        ValidateUnitInterval(settings.subpixelBlending,
            "FXAA subpixel blending must be finite and in [0, 1].");
    }

    std::optional<std::array<float, 2>> CalculateFxaaTexelSize(
        std::uint32_t sourceWidth,
        std::uint32_t sourceHeight) noexcept
    {
        if (sourceWidth == 0 || sourceHeight == 0)
        {
            return std::nullopt;
        }

        return std::array<float, 2>{
            1.0f / static_cast<float>(sourceWidth),
            1.0f / static_cast<float>(sourceHeight)
        };
    }
}
