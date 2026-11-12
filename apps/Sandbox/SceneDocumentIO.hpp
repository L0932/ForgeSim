#pragma once

#include "SceneDocument.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace ForgeSim::Sandbox
{
    struct MissingSceneAsset
    {
        std::string field;
        std::string reference;
        std::filesystem::path attemptedPath;
    };

    class MissingSceneAssets final : public std::runtime_error
    {
    public:
        MissingSceneAssets(std::filesystem::path root, std::vector<MissingSceneAsset> assets,
            const std::filesystem::path& scene = {});
        std::filesystem::path root;
        std::vector<MissingSceneAsset> assets;
    };

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