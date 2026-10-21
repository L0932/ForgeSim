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
                { "scale", WriteVector(transform.scale) }
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
                    property + ".scale")
            };
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

        const Json root{
            { "version", document.version },
            { "name", document.name },
            { "objects", objects },
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

        if (version != CurrentSceneDocumentVersion)
        {
            ReadError(
                "version",
                "Unsupported scene document version: " +
                std::to_string(version) + ".");
        }

        SceneDocument document;
        document.version = static_cast<std::uint32_t>(version);

        document.name = ReadString(
            ReadMember(root, "name", ""),
            "name");

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

        if (const auto error = ValidateSceneDocument(document))
        {
            ReadError(error->property, error->message);
        }

        return document;
    }
}