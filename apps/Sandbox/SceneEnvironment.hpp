#pragma once

#include "SceneRenderingSettings.hpp"

#include <filesystem>
#include <optional>

namespace ForgeSim::Sandbox
{
    // nullopt requests a black environment with no IBL contribution.
    [[nodiscard]] std::optional<std::filesystem::path> ResolveSceneEnvironment(
        const SceneRenderingSettings& settings,
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& bundledEnvironmentDirectory);
}
