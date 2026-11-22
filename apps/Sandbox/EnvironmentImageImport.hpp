#pragma once

#include "SceneAssetPath.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

namespace ForgeSim::Sandbox
{
    // Keep scene references portable. Files already in the asset root are reused;
    // external files are copied without overwriting existing assets. If loading
    // fails, remove only the copy made by this operation.
    template<typename Apply>
    void ImportEnvironmentImage(const std::filesystem::path& selected,
        const std::filesystem::path& projectRoot, Apply&& apply)
    {
        namespace fs = std::filesystem;
        const auto root = fs::canonical(projectRoot);
        const auto source = fs::canonical(selected);
        if (!fs::is_directory(root) || !fs::is_regular_file(source))
            throw std::runtime_error("Choose an accessible HDR image and asset root.");
        const auto reference = [&](const fs::path& file)
        {
            const auto bytes = file.lexically_relative(root).generic_u8string();
            return std::string{bytes.begin(), bytes.end()};
        };
        auto asset = reference(source);
        if (IsValidSceneAssetPath(asset))
        {
            apply(asset);
            return;
        }

        const auto directory = root / "environments";
        fs::create_directories(directory);
        const auto destinationDirectory = fs::canonical(directory);
        if (!IsValidSceneAssetPath(reference(destinationDirectory)))
            throw std::runtime_error("Environment folder must be inside the asset root.");
        for (unsigned suffix = 0; suffix < 10000; ++suffix)
        {
            const auto name = suffix == 0 ? source.filename() :
                fs::path{source.stem().native() + fs::path{"_" + std::to_string(suffix)}.native() + source.extension().native()};
            const auto destination = destinationDirectory / name;
            asset = reference(destination);
            if (!IsValidSceneAssetPath(asset))
                throw std::runtime_error("Image filename cannot be stored as a portable asset path.");
            std::error_code copyError;
            if (!fs::copy_file(source, destination, fs::copy_options::none, copyError))
            {
                if (copyError == std::errc::file_exists) continue;
                throw fs::filesystem_error("Cannot copy environment image", source, destination, copyError);
            }
            try { apply(asset); }
            catch (...)
            {
                std::error_code ignored;
                fs::remove(destination, ignored);
                throw;
            }
            return;
        }
        throw std::runtime_error("Could not choose an unused environment image filename.");
    }
}
