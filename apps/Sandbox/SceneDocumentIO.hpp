#pragma once

#include "SceneDocument.hpp"

#include <filesystem>

namespace ForgeSim::Sandbox
{
    // Model paths resolve relative to projectRoot, not the scene file.
    // Throws std::runtime_error with file context on failure.
    [[nodiscard]] SceneDocument LoadSceneDocument(
        const std::filesystem::path& scenePath,
        const std::filesystem::path& projectRoot);

    // Requires an existing destination directory.
    // Model references resolve against projectRoot.
    // Throws std::runtime_error with file context on failure.
    void SaveSceneDocument(
        const SceneDocument& document,
        const std::filesystem::path& scenePath,
        const std::filesystem::path& projectRoot);
}