#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct ExternalSampleLocation
    {
        std::filesystem::path scene;
        std::filesystem::path root;
    };

    [[nodiscard]] inline std::filesystem::path SamplePathFromUtf8(std::string_view text)
    {
        return std::filesystem::path{std::u8string{text.begin(), text.end()}};
    }

    // Text entered in ImGui is UTF-8, including on Windows.
    [[nodiscard]] inline ExternalSampleLocation ResolveExternalSample(
        std::string_view sceneText, std::string_view rootText)
    {
        if (sceneText.empty() || rootText.empty())
            throw std::invalid_argument("Choose a scene file and project root.");
        const auto root = SamplePathFromUtf8(rootText);
        if (!root.is_absolute())
            throw std::invalid_argument("Project root must be an absolute folder path.");
        if (!std::filesystem::is_directory(root))
            throw std::invalid_argument("Project root folder does not exist.");
        const auto enteredScene = SamplePathFromUtf8(sceneText);
        const auto scene = enteredScene.is_absolute() ? enteredScene : root / enteredScene;
        if (!std::filesystem::is_regular_file(scene))
            throw std::invalid_argument("Scene file does not exist or is not accessible.");
        if (scene.extension() != ".json")
            throw std::invalid_argument("Choose a ForgeSim scene JSON file, not a model file.");
        return {std::filesystem::canonical(scene), std::filesystem::canonical(root)};
    }
}
