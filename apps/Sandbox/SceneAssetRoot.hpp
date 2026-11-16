#pragma once

#include <filesystem>
#include <optional>

namespace ForgeSim::Sandbox
{
    enum class SceneAssetRootSource { ExplicitOverride, ProjectManifest, LegacyFallback };

    struct SceneAssetRoot
    {
        std::filesystem::path root;
        SceneAssetRootSource source;
        std::optional<std::filesystem::path> manifest;
    };

    // Scene paths are resolved before discovery. The fallback is supplied by the
    // caller (normally the application directory), never inferred from the cwd.
    // Throws on an inaccessible scene/root or an invalid nearest manifest.
    [[nodiscard]] SceneAssetRoot ResolveSceneAssetRoot(
        const std::filesystem::path& scenePath,
        const std::optional<std::filesystem::path>& explicitRoot,
        const std::filesystem::path& legacyRoot);
}
