#pragma once

#include "SceneAssetPath.hpp"
#include "SceneDocumentIO.hpp"

#include <optional>
#include <string_view>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline std::string RecoveryPathText(const std::filesystem::path& path)
    {
        const auto text = path.generic_u8string();
        return {text.begin(), text.end()};
    }

    [[nodiscard]] inline std::string DescribeMissingSceneAssets(
        const std::filesystem::path& scene, const MissingSceneAssets& missing)
    {
        std::string text = "Scene: " + RecoveryPathText(scene) +
            "\nAsset folder: " + RecoveryPathText(missing.root);
        for (const auto& asset : missing.assets)
        {
            text += asset.field == "rendering.environment.asset" ? "\n\nMissing environment: " : "\n\nMissing model: ";
            text += asset.reference + "\nLooked here: " + RecoveryPathText(asset.attemptedPath);
        }
        return text;
    }

    // Locate a model to infer its root, without rewriting its authored reference.
    // Validate the complete suffix through filesystem identity, including the
    // platform's case sensitivity. Unrelated/renamed subfolders cannot establish
    // a valid root for an unchanged scene reference.
    [[nodiscard]] inline std::optional<std::filesystem::path> InferSceneAssetRoot(
        const std::filesystem::path& selectedFile, std::string_view reference)
    {
        if (!IsValidSceneAssetPath(reference)) return std::nullopt;
        const std::filesystem::path relative{std::u8string{reference.begin(), reference.end()}};
        const auto selected = std::filesystem::canonical(selectedFile);
        if (!std::filesystem::is_regular_file(selected)) return std::nullopt;
        auto root = selected;
        for (const auto& component : relative)
        {
            static_cast<void>(component);
            const auto parent = root.parent_path();
            if (parent == root) return std::nullopt;
            root = parent;
        }
        std::error_code error;
        if (!std::filesystem::equivalent(root / relative, selected, error) || error)
            return std::nullopt;
        return root;
    }
}
