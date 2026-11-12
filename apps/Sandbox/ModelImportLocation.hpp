#pragma once

#include "SceneAssetRoot.hpp"
#include "SceneAssetPath.hpp"

#include <stdexcept>
#include <string>

namespace ForgeSim::Sandbox
{
    struct ModelImportLocation
    {
        std::filesystem::path model;
        std::filesystem::path root;
    };

    // The caller resolves the CLI's bundled-relative/absolute filename policy.
    // Standalone models without a project use their containing folder as root.
    [[nodiscard]] inline ModelImportLocation ResolveModelImportLocation(
        const std::filesystem::path& modelPath,
        const std::optional<std::filesystem::path>& explicitRoot = std::nullopt)
    {
        namespace fs = std::filesystem;
        const auto model = fs::canonical(modelPath);
        const auto root = ResolveSceneAssetRoot(model, explicitRoot, model.parent_path()).root;
        auto relative = model.filename();
        auto parent = model.parent_path();
        // Filesystem identity respects Windows case rules without changing the
        // authored asset spelling. Construct a reference only below this root.
        while (!fs::equivalent(parent, root))
        {
            const auto next = parent.parent_path();
            if (next == parent)
                throw std::invalid_argument("The model is outside the selected asset root. Choose a root containing the model, or omit --project-root for automatic discovery.");
            relative = parent.filename() / relative;
            parent = next;
        }
        const auto bytes = relative.generic_u8string();
        const std::string reference(bytes.begin(), bytes.end());
        if (!IsValidSceneAssetPath(reference))
            throw std::invalid_argument("The model path cannot be stored as a portable scene reference.");
        return {root / relative, root};
    }
}
