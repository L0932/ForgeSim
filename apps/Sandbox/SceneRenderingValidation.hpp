#pragma once

#include "SceneRenderingSettings.hpp"

#include <optional>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct RenderingValidationError
    {
        std::string_view property;
        std::string_view message;
    };

    [[nodiscard]] std::optional<RenderingValidationError>
        ValidateSceneRendering(
            const SceneRenderingSettings& settings) noexcept;
}