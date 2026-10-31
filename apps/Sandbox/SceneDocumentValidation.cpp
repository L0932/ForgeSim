#include "SceneCameraValidation.hpp"
#include "SceneDocumentValidation.hpp"
#include "SceneLightValidation.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace ForgeSim::Sandbox
{
    namespace
    {
        using Error = SceneDocumentValidationError;

        [[nodiscard]] bool IsFinite(const glm::vec3& value)
        {
            return std::isfinite(value.x) &&
                std::isfinite(value.y) &&
                std::isfinite(value.z);
        }

        [[nodiscard]] bool IsValidModelPath(std::string_view path)
        {
            if (path.empty())
            {
                return false;
            }

            // Reject platform-specific separators, drive prefixes,
            // embedded nulls, and other control characters.
            for (const char character : path)
            {
                const auto byte =
                    static_cast<unsigned char>(character);

                if (character == '\\' ||
                    character == ':' ||
                    byte < 32 ||
                    byte == 127)
                {
                    return false;
                }
            }

            // Require nonempty, ordinary relative components.
            // This also rejects leading/trailing or repeated slashes.
            std::size_t begin = 0;

            while (begin < path.size())
            {
                const std::size_t end = path.find('/', begin);

                const std::string_view component =
                    path.substr(
                        begin,
                        end == std::string_view::npos
                        ? path.size() - begin
                        : end - begin);

                if (component.empty() ||
                    component == "." ||
                    component == "..")
                {
                    return false;
                }

                if (end == std::string_view::npos)
                {
                    return true;
                }

                begin = end + 1;
            }

            return false;
        }

        [[nodiscard]] std::optional<Error> ValidateTransform(
            const Transform& transform,
            const std::string& property)
        {
            if (!IsFinite(transform.position))
            {
                return Error{
                    property + ".position",
                    "Position components must be finite."
                };
            }

            if (!IsFinite(transform.rotationRadians))
            {
                return Error{
                    property + ".rotationRadians",
                    "Rotation components must be finite."
                };
            }

            if (!IsFinite(transform.scale) ||
                transform.scale.x == 0.0f ||
                transform.scale.y == 0.0f ||
                transform.scale.z == 0.0f)
            {
                return Error{
                    property + ".scale",
                    "Scale components must be finite and nonzero."
                };
            }

            return std::nullopt;
        }

        template<typename Light>
        [[nodiscard]] std::optional<Error> ValidateDocumentLight(
            const Light& light,
            const std::string& property,
            std::unordered_set<SceneLightId>& lightIds)
        {
            if (light.id == InvalidSceneLightId)
            {
                return Error{
                    property + ".id",
                    "Light ID must be nonzero."
                };
            }

            if (!lightIds.insert(light.id).second)
            {
                return Error{
                    property + ".id",
                    "Light ID is already used by another light."
                };
            }

            if (const auto error = ValidateLight(light))
            {
                return Error{
                    property + "." + std::string{ error->property },
                    std::string{ error->message }
                };
            }

            return std::nullopt;
        }
    }

    std::optional<SceneDocumentValidationError>
        ValidateSceneDocument(const SceneDocument& document)
    {
        if (document.version != CurrentSceneDocumentVersion)
        {
            return Error{
                "version",
                "Unsupported scene document version: " +
                    std::to_string(document.version) + "."
            };
        }

        if (document.name.empty())
        {
            return Error{
                "name",
                "Scene name must not be empty."
            };
        }

        if (document.camera)
        {
            if (const auto error = ValidateSceneCamera(*document.camera))
            {
                return Error{
                    "camera." + std::string{ error->property },
                    std::string{ error->message }
                };
            }
        }

        std::unordered_set<std::uint64_t> objectIds;

        for (std::size_t index = 0;
            index < document.objects.size();
            ++index)
        {
            const SceneObjectData& object =
                document.objects[index];

            const std::string property =
                "objects[" + std::to_string(index) + "]";

            if (object.id == 0)
            {
                return Error{
                    property + ".id",
                    "Object ID must be nonzero."
                };
            }

            if (!objectIds.insert(object.id).second)
            {
                return Error{
                    property + ".id",
                    "Object ID is already used by another object."
                };
            }

            if (object.name.empty())
            {
                return Error{
                    property + ".name",
                    "Object name must not be empty."
                };
            }

            if (auto error = ValidateTransform(
                object.transform,
                property + ".transform"))
            {
                return error;
            }

            if (!IsValidModelPath(object.modelPath))
            {
                return Error{
                    property + ".modelPath",
                    "Model path must be project-relative, use forward "
                    "slashes, and contain no empty, '.' or '..' components."
                };
            }
        }

        std::unordered_set<SceneLightId> lightIds;

        if (auto error = ValidateDocumentLight(
            document.directionalLight,
            "directionalLight",
            lightIds))
        {
            return error;
        }

        for (std::size_t index = 0;
            index < document.pointLights.size();
            ++index)
        {
            if (auto error = ValidateDocumentLight(
                document.pointLights[index],
                "pointLights[" + std::to_string(index) + "]",
                lightIds))
            {
                return error;
            }
        }

        for (std::size_t index = 0;
            index < document.spotLights.size();
            ++index)
        {
            if (auto error = ValidateDocumentLight(
                document.spotLights[index],
                "spotLights[" + std::to_string(index) + "]",
                lightIds))
            {
                return error;
            }
        }

        return std::nullopt;
    }
}