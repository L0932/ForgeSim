#include "SceneRenderingValidation.hpp"
#include "SceneAssetPath.hpp"

#include <cmath>

namespace ForgeSim::Sandbox
{
    std::optional<RenderingValidationError>
        ValidateSceneRendering(
            const SceneRenderingSettings& settings) noexcept
    {
        const auto validColorComponent = [](float value)
            {
                return std::isfinite(value) &&
                    value >= 0.0f &&
                    value <= 1.0f;
            };

        if (!validColorComponent(settings.backgroundColor.r) ||
            !validColorComponent(settings.backgroundColor.g) ||
            !validColorComponent(settings.backgroundColor.b))
        {
            return RenderingValidationError{
                .property = "backgroundColor",
                .message =
                    "Background color components must be finite "
                    "and between zero and one."
            };
        }

        if (!std::isfinite(settings.exposure) ||
            settings.exposure <= 0.0f)
        {
            return RenderingValidationError{
                .property = "exposure",
                .message =
                    "Exposure must be finite and greater than zero."
            };
        }

        if (!std::isfinite(settings.environmentIntensity) ||
            settings.environmentIntensity < 0.0f)
        {
            return RenderingValidationError{
                .property = "environmentIntensity",
                .message =
                    "Environment intensity must be finite "
                    "and nonnegative."
            };
        }

        const DirectionalShadowSettings& shadows =
            settings.directionalShadows;

        if (!std::isfinite(shadows.minimumBias) ||
            shadows.minimumBias < 0.0f ||
            shadows.minimumBias > 0.01f)
        {
            return RenderingValidationError{
                .property = "directionalShadows.minimumBias",
                .message =
                    "Minimum shadow bias must be finite "
                    "and between zero and 0.01."
            };
        }

        if (!std::isfinite(shadows.slopeBias) ||
            shadows.slopeBias < 0.0f ||
            shadows.slopeBias > 0.02f)
        {
            return RenderingValidationError{
                .property = "directionalShadows.slopeBias",
                .message =
                    "Slope shadow bias must be finite "
                    "and between zero and 0.02."
            };
        }

        switch (settings.environmentSource)
        {
        case SceneEnvironmentSource::BundledStudio:
        case SceneEnvironmentSource::None:
            if (!settings.environmentAsset.empty())
            {
                return RenderingValidationError{
                    "environment.asset",
                    "Only project environments may have an asset reference."
                };
            }
            break;
        case SceneEnvironmentSource::ProjectAsset:
            if (!IsValidSceneAssetPath(settings.environmentAsset))
            {
                return RenderingValidationError{
                    "environment.asset",
                    "Environment path must be project-relative, use forward "
                    "slashes, and contain no empty, '.' or '..' components."
                };
            }
            break;
        default:
            return RenderingValidationError{
                "environment.source", "Unknown environment source."
            };
        }

        return std::nullopt;
    }
}