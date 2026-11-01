#pragma once

#include "SceneCameraData.hpp"

#include <optional>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct CameraValidationError
    {
        std::string_view property;
        std::string_view message;
    };

    [[nodiscard]] std::optional<CameraValidationError>
    ValidateCameraBookmarkName(std::string_view name) noexcept;

    // Empty optional means valid. Never modifies camera data.
    [[nodiscard]]
    std::optional<CameraValidationError> ValidateSceneCamera(
        const SceneCameraData& camera) noexcept;
}