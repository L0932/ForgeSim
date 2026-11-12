#include "SceneSerializer.hpp"
#include "SceneDocumentValidation.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        using Json = nlohmann::json;

        [[nodiscard]] Json WriteVector(const glm::vec3& value)
        {
            return Json::array({
                value.x,
                value.y,
                value.z
                });
        }

        [[nodiscard]] Json WriteTransform(
            const Transform& transform)
        {
            return Json{
                { "position", WriteVector(transform.position) },
                {
                    "rotationRadians",
                    WriteVector(transform.rotationRadians)
                },
                { "scale", WriteVector(transform.scale) },
                { "pivot", WriteVector(transform.pivot) }
            };
        }

        [[nodiscard]] Json WriteCamera(const SceneCameraData& camera)
        {
            Json result{
                { "position", WriteVector(camera.position) },
                {
                    "forwardDirection",
                    WriteVector(camera.forwardDirection)
                },
                { "upDirection", WriteVector(camera.upDirection) },
                {
                    "verticalFieldOfViewRadians",
                    camera.verticalFieldOfViewRadians
                },
                { "nearClippingPlane", camera.nearClippingPlane },
                { "farClippingPlane", camera.farClippingPlane }
            };
            result["referenceViewport"] = camera.referenceViewport
                ? Json{ { "width", camera.referenceViewport->width },
                    { "height", camera.referenceViewport->height } }
                : Json(nullptr);
            return result;
        }

        [[nodiscard]] Json WriteDirectionalShadows(
            const DirectionalShadowSettings& settings)
        {
            return Json{
                { "enabled", settings.enabled },
                { "filteringEnabled", settings.filteringEnabled },
                { "minimumBias", settings.minimumBias },
                { "slopeBias", settings.slopeBias }
            };
        }

        [[nodiscard]] Json WriteRendering(
            const SceneRenderingSettings& settings)
        {
            return Json{
                {
                    "backgroundColor",
                    WriteVector(settings.backgroundColor)
                },
                { "exposure", settings.exposure },
                {
                    "environmentIntensity",
                    settings.environmentIntensity
                },
                {
                    "showEnvironmentBackground",
                    settings.showEnvironmentBackground
                },
                {
                    "directionalShadows",
                    WriteDirectionalShadows(settings.directionalShadows)
                },
                { "environment", {
                    { "source", settings.environmentSource == SceneEnvironmentSource::BundledStudio
                        ? "bundledStudio" : settings.environmentSource == SceneEnvironmentSource::None
                            ? "none" : "projectAsset" },
                    { "asset", settings.environmentSource == SceneEnvironmentSource::ProjectAsset
                        ? Json(settings.environmentAsset) : Json(nullptr) }
                } },
            };
        }

        [[nodiscard]] Json WriteObject(
            const SceneObjectData& object)
        {
            return Json{
                { "id", object.id },
                { "name", object.name },
                { "transform", WriteTransform(object.transform) },
                { "modelPath", object.modelPath }
            };
        }

        template<typename Light>
        [[nodiscard]] Json WriteCommonLight(const Light& light)
        {
            return Json{
                { "id", light.id },
                { "enabled", light.enabled },
                { "position", WriteVector(light.position) },
                { "color", WriteVector(light.color) },
                { "intensity", light.intensity }
            };
        }

        [[nodiscard]] Json WriteLight(
            const DirectionalLight& light)
        {
            Json result = WriteCommonLight(light);

            result["directionToLight"] =
                WriteVector(light.directionToLight);

            return result;
        }

        [[nodiscard]] Json WriteLight(const PointLight& light)
        {
            Json result = WriteCommonLight(light);

            result["range"] = light.range;

            return result;
        }

        [[nodiscard]] Json WriteLight(const SpotLight& light)
        {
            Json result = WriteCommonLight(light);

            result["emissionDirection"] =
                WriteVector(light.emissionDirection);

            result["range"] = light.range;

            result["innerConeAngleRadians"] =
                light.innerConeAngleRadians;

            result["outerConeAngleRadians"] =
                light.outerConeAngleRadians;

            return result;
        }

        [[noreturn]] void ReadError(
            const std::string& property,
            const std::string& message)
        {
            throw std::invalid_argument(
                "Cannot deserialize scene document at '" +
                property + "': " + message);
        }

        [[nodiscard]] const Json& ReadMember(
            const Json& object,
            const char* name,
            const std::string& parent)
        {
            if (!object.is_object())
            {
                ReadError(parent, "Expected an object.");
            }

            const std::string property =
                parent.empty()
                ? std::string{ name }
            : parent + "." + name;

            const auto iterator = object.find(name);

            if (iterator == object.end())
            {
                ReadError(property, "Required field is missing.");
            }

            return *iterator;
        }

        [[nodiscard]] const Json& ReadArray(
            const Json& value,
            const std::string& property)
        {
            if (!value.is_array())
            {
                ReadError(property, "Expected an array.");
            }

            return value;
        }

        [[nodiscard]] std::string ReadString(
            const Json& value,
            const std::string& property)
        {
            if (!value.is_string())
            {
                ReadError(property, "Expected a string.");
            }

            return value.get<std::string>();
        }

        [[nodiscard]] bool ReadBool(
            const Json& value,
            const std::string& property)
        {
            if (!value.is_boolean())
            {
                ReadError(property, "Expected a boolean.");
            }

            return value.get<bool>();
        }

        [[nodiscard]] std::uint64_t ReadUnsigned(
            const Json& value,
            const std::string& property)
        {
            // Test unsigned first: positive JSON integers may occupy
            // the full uint64_t range.
            if (value.is_number_unsigned())
            {
                return value.get<std::uint64_t>();
            }

            if (value.is_number_integer())
            {
                const auto number = value.get<std::int64_t>();

                if (number >= 0)
                {
                    return static_cast<std::uint64_t>(number);
                }
            }

            ReadError(
                property,
                "Expected a nonnegative integer within uint64_t range.");
        }

        [[nodiscard]] float ReadFloat(
            const Json& value,
            const std::string& property)
        {
            if (!value.is_number())
            {
                ReadError(property, "Expected a number.");
            }

            const double number = value.get<double>();

            constexpr double maximum =
                static_cast<double>(
                    std::numeric_limits<float>::max());

            if (!std::isfinite(number) ||
                number < -maximum ||
                number > maximum)
            {
                ReadError(
                    property,
                    "Number must be finite and within float range.");
            }

            return static_cast<float>(number);
        }

        [[nodiscard]] glm::vec3 ReadVector(
            const Json& value,
            const std::string& property)
        {
            if (!value.is_array() || value.size() != 3)
            {
                ReadError(
                    property,
                    "Expected an array of exactly three numbers.");
            }

            return glm::vec3{
                ReadFloat(value[0], property + "[0]"),
                ReadFloat(value[1], property + "[1]"),
                ReadFloat(value[2], property + "[2]")
            };
        }

        [[nodiscard]] Transform ReadTransform(
            const Json& value,
            const std::string& property)
        {
            return Transform{
                .position = ReadVector(
                    ReadMember(value, "position", property),
                    property + ".position"),

                .rotationRadians = ReadVector(
                    ReadMember(value, "rotationRadians", property),
                    property + ".rotationRadians"),

                .scale = ReadVector(
                    ReadMember(value, "scale", property),
                    property + ".scale"),

                .pivot = value.contains("pivot")
                    ? ReadVector(value.at("pivot"), property + ".pivot")
                    : glm::vec3{0.0f}
            };
        }

        [[nodiscard]] SceneCameraData ReadCamera(
            const Json& value,
            const std::string& property)
        {
            SceneCameraData camera{
                .position = ReadVector(
                    ReadMember(value, "position", property),
                    property + ".position"),

                .forwardDirection = ReadVector(
                    ReadMember(value, "forwardDirection", property),
                    property + ".forwardDirection"),

                .upDirection = ReadVector(
                    ReadMember(value, "upDirection", property),
                    property + ".upDirection"),

                .verticalFieldOfViewRadians = ReadFloat(
                    ReadMember(value, "verticalFieldOfViewRadians", property),
                    property + ".verticalFieldOfViewRadians"),

                .nearClippingPlane = ReadFloat(
                    ReadMember(value, "nearClippingPlane", property),
                    property + ".nearClippingPlane"),

                .farClippingPlane = ReadFloat(
                    ReadMember(value, "farClippingPlane", property),
                    property + ".farClippingPlane")
            };
            const auto extent = value.find("referenceViewport");
            if (extent != value.end() && !extent->is_null())
            {
                const auto width = ReadUnsigned(
                    ReadMember(*extent, "width", property + ".referenceViewport"),
                    property + ".referenceViewport.width");
                const auto height = ReadUnsigned(
                    ReadMember(*extent, "height", property + ".referenceViewport"),
                    property + ".referenceViewport.height");
                if (width == 0 || width > std::numeric_limits<std::uint32_t>::max())
                    ReadError(property + ".referenceViewport.width", "Expected a positive uint32 dimension.");
                if (height == 0 || height > std::numeric_limits<std::uint32_t>::max())
                    ReadError(property + ".referenceViewport.height", "Expected a positive uint32 dimension.");
                camera.referenceViewport = SceneCaptureExtent{
                    static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };
            }
            return camera;
        }

        [[nodiscard]] DirectionalShadowSettings ReadDirectionalShadows(
            const Json& value,
            const std::string& property)
        {
            return DirectionalShadowSettings{
                .enabled = ReadBool(
                    ReadMember(value, "enabled", property),
                    property + ".enabled"),

                .filteringEnabled = ReadBool(
                    ReadMember(value, "filteringEnabled", property),
                    property + ".filteringEnabled"),

                .minimumBias = ReadFloat(
                    ReadMember(value, "minimumBias", property),
                    property + ".minimumBias"),

                .slopeBias = ReadFloat(
                    ReadMember(value, "slopeBias", property),
                    property + ".slopeBias")
            };
        }

        [[nodiscard]] SceneRenderingSettings ReadRendering(
            const Json& value,
            const std::string& property)
        {
            SceneRenderingSettings settings{
                .backgroundColor = ReadVector(
                    ReadMember(value, "backgroundColor", property),
                    property + ".backgroundColor"),

                .exposure = ReadFloat(
                    ReadMember(value, "exposure", property),
                    property + ".exposure"),

                .environmentIntensity = ReadFloat(
                    ReadMember(value, "environmentIntensity", property),
                    property + ".environmentIntensity"),

                .showEnvironmentBackground = ReadBool(
                    ReadMember(value, "showEnvironmentBackground", property),
                    property + ".showEnvironmentBackground")
            };

            const auto shadows = value.find("directionalShadows");

            if (shadows != value.end())
            {
                settings.directionalShadows = ReadDirectionalShadows(
                    *shadows,
                    property + ".directionalShadows");
            }

            const auto environment = value.find("environment");
            if (environment != value.end())
            {
                const std::string parent = property + ".environment";
                const std::string source = ReadString(
                    ReadMember(*environment, "source", parent), parent + ".source");
                const Json& asset = ReadMember(*environment, "asset", parent);
                if (source == "projectAsset")
                {
                    settings.environmentSource = SceneEnvironmentSource::ProjectAsset;
                    settings.environmentAsset = ReadString(asset, parent + ".asset");
                }
                else
                {
                    if (source == "bundledStudio")
                        settings.environmentSource = SceneEnvironmentSource::BundledStudio;
                    else if (source == "none")
                        settings.environmentSource = SceneEnvironmentSource::None;
                    else
                        ReadError(parent + ".source", "Unknown environment source.");
                    if (!asset.is_null())
                        ReadError(parent + ".asset", "Expected null for a non-project environment.");
                }
            }
            return settings;
        }

        [[nodiscard]] SceneObjectData ReadObject(
            const Json& value,
            const std::string& property)
        {
            return SceneObjectData{
                .id = ReadUnsigned(
                    ReadMember(value, "id", property),
                    property + ".id"),

                .name = ReadString(
                    ReadMember(value, "name", property),
                    property + ".name"),

                .transform = ReadTransform(
                    ReadMember(value, "transform", property),
                    property + ".transform"),

                .modelPath = ReadString(
                    ReadMember(value, "modelPath", property),
                    property + ".modelPath")
            };
        }

        template<typename Light>
        [[nodiscard]] Light ReadCommonLight(
            const Json& value,
            const std::string& property)
        {
            Light light;

            light.id = ReadUnsigned(
                ReadMember(value, "id", property),
                property + ".id");

            light.enabled = ReadBool(
                ReadMember(value, "enabled", property),
                property + ".enabled");

            light.position = ReadVector(
                ReadMember(value, "position", property),
                property + ".position");

            light.color = ReadVector(
                ReadMember(value, "color", property),
                property + ".color");

            light.intensity = ReadFloat(
                ReadMember(value, "intensity", property),
                property + ".intensity");

            return light;
        }

        [[nodiscard]] DirectionalLight ReadDirectionalLight(
            const Json& value,
            const std::string& property)
        {
            DirectionalLight light =
                ReadCommonLight<DirectionalLight>(value, property);

            light.directionToLight = ReadVector(
                ReadMember(value, "directionToLight", property),
                property + ".directionToLight");

            return light;
        }

        [[nodiscard]] PointLight ReadPointLight(
            const Json& value,
            const std::string& property)
        {
            PointLight light =
                ReadCommonLight<PointLight>(value, property);

            light.range = ReadFloat(
                ReadMember(value, "range", property),
                property + ".range");

            return light;
        }

        [[nodiscard]] SpotLight ReadSpotLight(
            const Json& value,
            const std::string& property)
        {
            SpotLight light =
                ReadCommonLight<SpotLight>(value, property);

            light.emissionDirection = ReadVector(
                ReadMember(value, "emissionDirection", property),
                property + ".emissionDirection");

            light.range = ReadFloat(
                ReadMember(value, "range", property),
                property + ".range");

            light.innerConeAngleRadians = ReadFloat(
                ReadMember(value, "innerConeAngleRadians", property),
                property + ".innerConeAngleRadians");

            light.outerConeAngleRadians = ReadFloat(
                ReadMember(value, "outerConeAngleRadians", property),
                property + ".outerConeAngleRadians");

            return light;
        }
    }

    std::string SerializeSceneDocument(
        const SceneDocument& document)
    {
        if (const auto error = ValidateSceneDocument(document))
        {
            throw std::invalid_argument(
                "Cannot serialize scene document at '" +
                error->property + "': " + error->message);
        }

        Json objects = Json::array();

        for (const SceneObjectData& object : document.objects)
        {
            objects.push_back(WriteObject(object));
        }

        Json pointLights = Json::array();

        for (const PointLight& light : document.pointLights)
        {
            pointLights.push_back(WriteLight(light));
        }

        Json spotLights = Json::array();

        for (const SpotLight& light : document.spotLights)
        {
            spotLights.push_back(WriteLight(light));
        }

        Json bookmarks = Json::array();
        for (const auto& bookmark : document.cameraBookmarks)
        {
            bookmarks.push_back({ { "name", bookmark.name },
                { "camera", WriteCamera(bookmark.camera) } });
        }

        const Json root{
            { "version", document.version },
            { "name", document.name },
            {
              "camera",
                document.camera
                    ? WriteCamera(*document.camera)
                    : Json(nullptr)
            },
            { "rendering", WriteRendering(document.rendering) },
            { "cameraBookmarks", bookmarks },
            { "cameraMovementSpeed", document.cameraMovementSpeed ? Json(*document.cameraMovementSpeed) : Json(nullptr) },
            { "objects", objects },
            { "editorVisibility", {
                { "hiddenObjectIds", document.hiddenObjectIds },
                { "hiddenLightIds", document.hiddenLightIds }
            } },
            {
                "directionalLight",
                WriteLight(document.directionalLight)
            },
            { "pointLights", pointLights },
            { "spotLights", spotLights }
        };

        return root.dump(2) + "\n";
    }

    SceneDocument DeserializeSceneDocument(std::string_view text)
    {
        Json root;

        try
        {
            root = Json::parse(text.begin(), text.end());
        }
        catch (const Json::exception& exception)
        {
            throw std::invalid_argument(
                std::string{
                    "Cannot deserialize scene document: invalid JSON: "
                } + exception.what());
        }

        if (!root.is_object())
        {
            ReadError("root", "Expected an object.");
        }

        // Check the version before interpreting the remaining schema.
        const std::uint64_t version = ReadUnsigned(
            ReadMember(root, "version", ""),
            "version");

        if (version < 1 || version > CurrentSceneDocumentVersion)
        {
            ReadError(
                "version",
                "Unsupported scene document version: " +
                std::to_string(version) + ".");
        }

        // Older files are upgraded to the current in-memory representation.
        SceneDocument document;
        document.version = CurrentSceneDocumentVersion;

        document.name = ReadString(
            ReadMember(root, "name", ""),
            "name");

        if (version >= 2)
        {
            const Json& camera = ReadMember(root, "camera", "");

            if (!camera.is_null())
            {
                document.camera = ReadCamera(camera, "camera");
            }
        }

        const auto speed = root.find("cameraMovementSpeed");
        if (speed != root.end() && !speed->is_null())
            document.cameraMovementSpeed = ReadFloat(*speed, "cameraMovementSpeed");

        const auto bookmarks = root.find("cameraBookmarks");
        if (bookmarks != root.end())
        {
            const Json& entries = ReadArray(*bookmarks, "cameraBookmarks");
            for (std::size_t index = 0; index < entries.size(); ++index)
            {
                const std::string property = "cameraBookmarks[" + std::to_string(index) + "]";
                document.cameraBookmarks.push_back({
                    ReadString(ReadMember(entries[index], "name", property), property + ".name"),
                    ReadCamera(ReadMember(entries[index], "camera", property), property + ".camera")
                });
            }
        }

        const auto rendering = root.find("rendering");

        if (rendering != root.end())
        {
            document.rendering = ReadRendering(
                *rendering,
                "rendering");
        }

        const Json& objects = ReadArray(
            ReadMember(root, "objects", ""),
            "objects");

        document.objects.reserve(objects.size());

        for (std::size_t index = 0; index < objects.size(); ++index)
        {
            document.objects.push_back(
                ReadObject(
                    objects[index],
                    "objects[" + std::to_string(index) + "]"));
        }

        document.directionalLight = ReadDirectionalLight(
            ReadMember(root, "directionalLight", ""),
            "directionalLight");

        const Json& pointLights = ReadArray(
            ReadMember(root, "pointLights", ""),
            "pointLights");

        document.pointLights.reserve(pointLights.size());

        for (std::size_t index = 0;
            index < pointLights.size();
            ++index)
        {
            document.pointLights.push_back(
                ReadPointLight(
                    pointLights[index],
                    "pointLights[" + std::to_string(index) + "]"));
        }

        const Json& spotLights = ReadArray(
            ReadMember(root, "spotLights", ""),
            "spotLights");

        document.spotLights.reserve(spotLights.size());

        for (std::size_t index = 0;
            index < spotLights.size();
            ++index)
        {
            document.spotLights.push_back(
                ReadSpotLight(
                    spotLights[index],
                    "spotLights[" + std::to_string(index) + "]"));
        }

        if (const auto visibility = root.find("editorVisibility"); visibility != root.end())
        {
            if (!visibility->is_object()) ReadError("editorVisibility", "Expected an object.");
            const auto readIds = [&](const char* member, auto& destination)
            {
                if (const auto ids = visibility->find(member); ids != visibility->end())
                {
                    const std::string property = std::string("editorVisibility.") + member;
                    const auto& entries = ReadArray(*ids, property);
                    for (std::size_t index = 0; index < entries.size(); ++index)
                        destination.push_back(ReadUnsigned(entries[index],
                            property + "[" + std::to_string(index) + "]"));
                }
            };
            readIds("hiddenObjectIds", document.hiddenObjectIds);
            readIds("hiddenLightIds", document.hiddenLightIds);
        }

        if (const auto error = ValidateSceneDocument(document))
        {
            ReadError(error->property, error->message);
        }

        return document;
    }
}