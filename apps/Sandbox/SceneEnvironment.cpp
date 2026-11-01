#include "SceneEnvironment.hpp"
#include "SceneRenderingValidation.hpp"

#include <stdexcept>

namespace ForgeSim::Sandbox
{
    std::optional<std::filesystem::path> ResolveSceneEnvironment(
        const SceneRenderingSettings& settings,
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& bundledEnvironmentDirectory)
    {
        if (const auto error = ValidateSceneRendering(settings))
        {
            throw std::invalid_argument("rendering." + std::string{ error->property } +
                ": " + std::string{ error->message });
        }
        if (settings.environmentSource == SceneEnvironmentSource::None)
            return std::nullopt;

        const std::filesystem::path path =
            settings.environmentSource == SceneEnvironmentSource::BundledStudio
            ? bundledEnvironmentDirectory / "Studio_Small_01_4k.hdr"
            : projectRoot / std::filesystem::path{ std::u8string{
                settings.environmentAsset.begin(), settings.environmentAsset.end() } };
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error) || error)
        {
            throw std::runtime_error("rendering.environment.asset: Environment is not an accessible regular file: " +
                path.string());
        }
        return std::filesystem::absolute(path).lexically_normal();
    }
}
