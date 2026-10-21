#include "SceneSerializer.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace
{
    using namespace ForgeSim::Sandbox;
    using Json = nlohmann::json;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void TestDocumentOutput()
    {
        SceneDocument document;
        document.name = "Chapel \"Evening\"";

        // Exercise an ID above the exact integer range of a double.
        constexpr std::uint64_t objectId =
            9007199254740993ULL;

        document.objects.push_back(
            SceneObjectData{
                .id = objectId,
                .name = "Icon panel",
                .transform = Transform{
                    .position = { 1.0f, 2.0f, 3.0f },
                    .rotationRadians = { 0.25f, 0.5f, 0.75f },
                    .scale = { -1.0f, 2.0f, 0.5f }
                },
                .modelPath = "assets/models/Icon Panel.glb"
            });

        document.directionalLight.position = {
            4.0f, 5.0f, 6.0f
        };

        document.directionalLight.directionToLight = {
            0.0f, 2.0f, 0.0f
        };

        document.pointLights.push_back(
            PointLight{
                .id = 2,
                .enabled = false,
                .position = { 1.0f, 3.0f, 5.0f },
                .color = { 1.0f, 0.5f, 0.25f },
                .intensity = 8.0f,
                .range = 4.0f
            });

        document.spotLights.push_back(
            SpotLight{
                .id = 3,
                .position = { 2.0f, 4.0f, 6.0f },
                .emissionDirection = { 0.0f, -5.0f, 0.0f },
                .color = { 0.25f, 0.5f, 1.0f },
                .intensity = 12.0f,
                .range = 9.0f,
                .innerConeAngleRadians = 0.25f,
                .outerConeAngleRadians = 0.5f
            });

        const std::string text =
            SerializeSceneDocument(document);

        const Json json = Json::parse(text);

        Require(
            json.at("version").get<std::uint32_t>() ==
            CurrentSceneDocumentVersion,
            "Scene version was not preserved.");

        Require(
            json.at("name").get<std::string>() == document.name,
            "Scene name or escaping was not preserved.");

        const Json& object = json.at("objects").at(0);

        Require(
            object.at("id").is_number_unsigned() &&
            object.at("id").get<std::uint64_t>() == objectId,
            "Large object ID lost precision.");

        Require(
            object.at("name") == "Icon panel" &&
            object.at("modelPath") ==
            "assets/models/Icon Panel.glb",
            "Object metadata was not preserved.");

        const Json& transform = object.at("transform");

        Require(
            transform.at("position") ==
            Json::array({ 1.0f, 2.0f, 3.0f }) &&
            transform.at("rotationRadians") ==
            Json::array({ 0.25f, 0.5f, 0.75f }) &&
            transform.at("scale") ==
            Json::array({ -1.0f, 2.0f, 0.5f }),
            "Object transform was not preserved.");

        const Json& directional =
            json.at("directionalLight");

        Require(
            directional.at("id") == 1 &&
            directional.at("enabled") == true &&
            directional.at("position") ==
            Json::array({ 4.0f, 5.0f, 6.0f }) &&
            directional.at("directionToLight") ==
            Json::array({ 0.0f, 2.0f, 0.0f }) &&
            directional.at("color") ==
            Json::array({ 1.0f, 1.0f, 1.0f }) &&
            directional.at("intensity") == 3.0f,
            "Directional light data was not preserved.");

        const Json& point = json.at("pointLights").at(0);

        Require(
            point.at("id") == 2 &&
            point.at("enabled") == false &&
            point.at("position") ==
            Json::array({ 1.0f, 3.0f, 5.0f }) &&
            point.at("color") ==
            Json::array({ 1.0f, 0.5f, 0.25f }) &&
            point.at("intensity") == 8.0f &&
            point.at("range") == 4.0f,
            "Point light data was not preserved.");

        const Json& spot = json.at("spotLights").at(0);

        Require(
            spot.at("id") == 3 &&
            spot.at("enabled") == true &&
            spot.at("position") ==
            Json::array({ 2.0f, 4.0f, 6.0f }) &&
            spot.at("emissionDirection") ==
            Json::array({ 0.0f, -5.0f, 0.0f }) &&
            spot.at("color") ==
            Json::array({ 0.25f, 0.5f, 1.0f }) &&
            spot.at("intensity") == 12.0f &&
            spot.at("range") == 9.0f &&
            spot.at("innerConeAngleRadians") == 0.25f &&
            spot.at("outerConeAngleRadians") == 0.5f,
            "Spot light data was not preserved.");

        Require(
            text == SerializeSceneDocument(document),
            "Serializing the same document produced different output.");

        const SceneDocument restored =
            DeserializeSceneDocument(text);

        Require(
            SerializeSceneDocument(restored) == text,
            "Scene data changed during a JSON round trip.");
    }

    void TestEmptyArrays()
    {
        const Json json = Json::parse(
            SerializeSceneDocument(SceneDocument{}));

        for (const char* key : {
            "objects", "pointLights", "spotLights" })
        {
            Require(
                json.at(key).is_array() &&
                json.at(key).empty(),
                "Empty collections must serialize as arrays.");
        }
    }

    void TestInvalidDocumentRejected()
    {
        SceneDocument document;

        document.directionalLight.intensity =
            std::numeric_limits<float>::quiet_NaN();

        bool rejected = false;

        try
        {
            const std::string text =
                SerializeSceneDocument(document);

            static_cast<void>(text);
        }
        catch (const std::invalid_argument& exception)
        {
            rejected = true;

            Require(
                std::string{ exception.what() }.find(
                    "directionalLight.intensity") !=
                std::string::npos,
                "Serialization error omitted the property location.");
        }

        Require(
            rejected,
            "Invalid document was serialized.");
    }

    void ExpectReadError(
        std::string_view text,
        std::string_view expectedLocation)
    {
        bool rejected = false;

        try
        {
            const SceneDocument document =
                DeserializeSceneDocument(text);

            static_cast<void>(document);
        }
        catch (const std::invalid_argument& exception)
        {
            rejected = true;

            Require(
                std::string{ exception.what() }.find(
                    expectedLocation) != std::string::npos,
                "Read error omitted the expected location.");
        }

        Require(rejected, "Invalid scene JSON was accepted.");
    }

    void TestMalformedInput()
    {
        ExpectReadError("{", "invalid JSON");
        ExpectReadError("[]", "root");

        // Version is checked before the rest of the schema.
        ExpectReadError(R"({"version": 999})", "version");

        const Json valid = Json::parse(
            SerializeSceneDocument(SceneDocument{}));

        Json candidate = valid;
        candidate.erase("name");
        ExpectReadError(candidate.dump(), "name");

        candidate = valid;
        candidate["objects"] = Json::object();
        ExpectReadError(candidate.dump(), "objects");

        candidate = valid;
        candidate["directionalLight"]["enabled"] = 1;
        ExpectReadError(
            candidate.dump(),
            "directionalLight.enabled");

        candidate = valid;
        candidate["directionalLight"]["position"] =
            Json::array({ 1.0f, 2.0f });

        ExpectReadError(
            candidate.dump(),
            "directionalLight.position");

        candidate = valid;
        candidate["directionalLight"]["position"][1] = "two";
        ExpectReadError(
            candidate.dump(),
            "directionalLight.position[1]");

        candidate = valid;
        candidate["directionalLight"]["intensity"] = 1e100;
        ExpectReadError(
            candidate.dump(),
            "directionalLight.intensity");
    }

    void TestIdInput()
    {
        const Json valid = Json::parse(
            SerializeSceneDocument(SceneDocument{}));

        for (const Json& invalidId : Json::array({
            -1,
            1.5,
            1.0,
            "1",
            nullptr,
            1e30
            }))
        {
            Json candidate = valid;
            candidate["directionalLight"]["id"] = invalidId;

            ExpectReadError(
                candidate.dump(),
                "directionalLight.id");
        }

        Json candidate = valid;

        candidate["directionalLight"]["id"] =
            std::numeric_limits<std::uint64_t>::max();

        const SceneDocument document =
            DeserializeSceneDocument(candidate.dump());

        Require(
            document.directionalLight.id ==
            std::numeric_limits<std::uint64_t>::max(),
            "Maximum uint64_t ID was not preserved.");
    }

    void TestSemanticInputValidation()
    {
        SceneDocument source;

        source.pointLights.push_back(
            PointLight{
                .id = 2
            });

        const Json valid = Json::parse(
            SerializeSceneDocument(source));

        Json candidate = valid;
        candidate["pointLights"][0]["id"] =
            source.directionalLight.id;

        ExpectReadError(
            candidate.dump(),
            "pointLights[0].id");

        candidate = valid;
        candidate["pointLights"][0]["range"] = -1.0f;

        ExpectReadError(
            candidate.dump(),
            "pointLights[0].range");

        candidate = valid;
        candidate["directionalLight"]["directionToLight"] =
            Json::array({ 0.0f, 0.0f, 0.0f });

        ExpectReadError(
            candidate.dump(),
            "directionalLight.directionToLight");
    }

    void TestEmptyDocumentRoundTrip()
    {
        const SceneDocument restored =
            DeserializeSceneDocument(
                SerializeSceneDocument(SceneDocument{}));

        Require(
            restored.objects.empty() &&
            restored.pointLights.empty() &&
            restored.spotLights.empty(),
            "Empty collections changed during loading.");
    }
}

int main()
{
    try
    {
        TestDocumentOutput();
        TestEmptyArrays();
        TestInvalidDocumentRejected();
        TestMalformedInput();
        TestIdInput();
        TestSemanticInputValidation();
        TestEmptyDocumentRoundTrip();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}