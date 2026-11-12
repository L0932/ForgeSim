#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline std::filesystem::path SamplePathFromUtf8(std::string_view text)
    {
        return std::filesystem::path{std::u8string{text.begin(), text.end()}};
    }

    // Text entered in ImGui is UTF-8, including on Windows.
    [[nodiscard]] inline std::filesystem::path ResolveExternalSample(std::string_view sceneText)
    {
        if (sceneText.empty())
            throw std::invalid_argument("Choose a scene file.");
        const auto scene = SamplePathFromUtf8(sceneText);
        if (!scene.is_absolute())
            throw std::invalid_argument("Choose an absolute scene path, or use Browse scene.");
        if (!std::filesystem::is_regular_file(scene))
            throw std::invalid_argument("Scene file does not exist or is not accessible.");
        if (scene.extension() != ".json")
            throw std::invalid_argument("Choose a ForgeSim scene JSON file, not a model file.");
        return std::filesystem::canonical(scene);
    }
}
