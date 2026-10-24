#pragma once

#include <filesystem>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] std::filesystem::path BundledAssetDirectory();

    [[nodiscard]] std::filesystem::path ShaderDirectory();

    [[nodiscard]] std::filesystem::path EditorSettingsDirectory();

    [[nodiscard]] std::filesystem::path DefaultSceneDirectory();
}