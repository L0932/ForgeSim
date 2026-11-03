#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace ForgeSim::Renderer
{
    struct FxaaSettings
    {
        bool enabled = true;

        // Dimensionless controls in [0, 1]. Larger thresholds skip more
        // low-contrast edges; larger subpixel values allow more smoothing.
        float relativeContrastThreshold = 0.125f;
        float minimumContrastThreshold = 0.03125f;
        float subpixelBlending = 0.75f;
    };

    // All controls must be finite and in [0, 1], even when disabled.
    // Throws std::invalid_argument instead of silently clamping bad data.
    void ValidateFxaaSettings(const FxaaSettings& settings);

    // One source-image pixel expressed in normalized texture coordinates.
    // A zero-sized source requests that the caller skip rendering.
    // Use the actual source texture size, not the window or UI panel size.
    [[nodiscard]] std::optional<std::array<float, 2>> CalculateFxaaTexelSize(
        std::uint32_t sourceWidth,
        std::uint32_t sourceHeight) noexcept;
}
