#include "SceneAssetRoot.hpp"
#include "SceneAssetPath.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

namespace ForgeSim::Sandbox
{
    namespace
    {
        namespace fs = std::filesystem;

        fs::path ExistingDirectory(const fs::path& path)
        {
            const auto result = fs::canonical(path);
            if (!fs::is_directory(result))
                throw std::runtime_error("Asset root must be an existing directory: " + path.string());
            return result;
        }

        fs::path ReadManifestRoot(const fs::path& manifest)
        {
            try
            {
                std::ifstream input(manifest, std::ios::binary);
                if (!input) throw std::runtime_error("Cannot read project manifest.");
                const auto json = nlohmann::json::parse(input);
                if (input.bad()) throw std::runtime_error("Cannot finish reading project manifest.");
                if (!json.is_object() || !json.contains("version") ||
                    !json.at("version").is_number_integer() || json.at("version") != 1)
                    throw std::runtime_error("Expected project manifest version 1.");
                if (!json.contains("assetRoot") || !json.at("assetRoot").is_string())
                    throw std::runtime_error("assetRoot must be a relative folder path or '.'.");
                const auto text = json.at("assetRoot").get<std::string>();
                if (text != "." && !IsValidSceneAssetPath(text))
                    throw std::runtime_error("assetRoot must be '.' or a portable relative path without '..'.");
                const fs::path relative{std::u8string{text.begin(), text.end()}};
                return ExistingDirectory(manifest.parent_path() / relative);
            }
            catch (const std::exception& error)
            {
                throw std::runtime_error("Invalid project manifest '" + manifest.string() + "': " + error.what());
            }
        }
    }

    SceneAssetRoot ResolveSceneAssetRoot(
        const std::filesystem::path& scenePath,
        const std::optional<std::filesystem::path>& explicitRoot,
        const std::filesystem::path& legacyRoot)
    {
        const auto scene = fs::canonical(scenePath);
        if (!fs::is_regular_file(scene))
            throw std::runtime_error("Scene must be an existing regular file: " + scenePath.string());
        if (explicitRoot)
            return {ExistingDirectory(*explicitRoot), SceneAssetRootSource::ExplicitOverride, std::nullopt};

        auto directory = scene.parent_path();
        for (;;)
        {
            const auto manifest = directory / "ForgeSim.project.json";
            // symlink_status also detects broken links, which are invalid
            // manifests rather than a reason to adopt an unrelated parent.
            if (fs::exists(fs::symlink_status(manifest)))
                return {ReadManifestRoot(manifest), SceneAssetRootSource::ProjectManifest, manifest};
            const auto parent = directory.parent_path();
            if (parent == directory) break;
            directory = parent;
        }
        return {ExistingDirectory(legacyRoot), SceneAssetRootSource::LegacyFallback, std::nullopt};
    }
}
