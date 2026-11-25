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

        std::filesystem::path path;

        switch (settings.environmentSource)
        {
        case SceneEnvironmentSource::BundledStudio:
            path = bundledEnvironmentDirectory / "Studio_Small_01_4k.hdr";
            break;

        case SceneEnvironmentSource::BundledSky:
            path = bundledEnvironmentDirectory /
                "lonely_road_afternoon_puresky_4k.hdr";
            break;

        case SceneEnvironmentSource::ProjectAsset:
            path = projectRoot / std::filesystem::path{
                std::u8string{
                    settings.environmentAsset.begin(),
                    settings.environmentAsset.end()
                }
            };
            break;

        case SceneEnvironmentSource::None:
            return std::nullopt;

        default:
            throw std::runtime_error("Unknown environment source.");
        }

        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error) || error)
        {
            throw std::runtime_error("rendering.environment.asset: Environment is not an accessible regular file: " +
                path.string());
        }
        return std::filesystem::absolute(path).lexically_normal();
    }
}
