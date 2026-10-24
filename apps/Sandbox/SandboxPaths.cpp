#include "SandboxPaths.hpp"

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

#include <ForgeSim/Platform/ApplicationPaths.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        [[nodiscard]] std::string ShaderDirectoryOverride()
        {
#if defined(_WIN32)
            char* value = nullptr;
            std::size_t length = 0;

            const auto result = _dupenv_s(
                &value,
                &length,
                "FORGESIM_SHADER_DIRECTORY");

            const std::unique_ptr<char, decltype(&std::free)> owner{
                value,
                &std::free
            };

            if (result != 0)
            {
                throw std::runtime_error(
                    "Cannot read FORGESIM_SHADER_DIRECTORY.");
            }

            return value != nullptr ? std::string{ value } : std::string{};
#else
            const char* value =
                std::getenv("FORGESIM_SHADER_DIRECTORY");

            return value != nullptr ? std::string{ value } : std::string{};
#endif
        }
    }

    std::filesystem::path BundledAssetDirectory()
    {
        return Platform::ExecutableDirectory() / "assets";
    }

    std::filesystem::path ShaderDirectory()
    {
        const std::string overridePath = ShaderDirectoryOverride();

        const std::filesystem::path directory =
            overridePath.empty()
            ? BundledAssetDirectory() / "shaders"
            : std::filesystem::absolute(
                std::filesystem::path{ overridePath });

        if (!std::filesystem::is_directory(directory))
        {
            throw std::runtime_error(
                "Shader directory does not exist: " +
                directory.string());
        }

        return directory.lexically_normal();
    }

    std::filesystem::path EditorSettingsDirectory()
    {
        const std::filesystem::path directory =
            Platform::UserDataDirectory() / "ForgeSim" / "Settings";

        std::filesystem::create_directories(directory);

        return directory;
    }

    std::filesystem::path DefaultSceneDirectory()
    {
        const std::filesystem::path directory =
            Platform::UserDataDirectory() / "ForgeSim" / "Scenes";

        std::filesystem::create_directories(directory);

        return directory;
    }
}