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

    void TestCameraInputErrors()
    {
        SceneDocument source;
        source.camera = SceneCameraData{};

        const Json valid =
            Json::parse(SerializeSceneDocument(source));

        Json candidate = valid;
        candidate.erase("camera");
        ExpectReadError(candidate.dump(), "camera");

        candidate = valid;
        candidate["camera"] = Json::array();
        ExpectReadError(candidate.dump(), "camera");

        candidate = valid;
        candidate["camera"].erase("position");
        ExpectReadError(candidate.dump(), "camera.position");

        candidate = valid;
        candidate["camera"]["nearClippingPlane"] = "near";
        ExpectReadError(
            candidate.dump(),
            "camera.nearClippingPlane");

        candidate = valid;
        candidate["camera"]["nearClippingPlane"] = 0.0f;
        ExpectReadError(
            candidate.dump(),
            "camera.nearClippingPlane");

        candidate = valid;
        candidate["camera"]["farClippingPlane"] =
            source.camera->nearClippingPlane;
        ExpectReadError(
            candidate.dump(),
            "camera.farClippingPlane");
    }

    void TestCameraRoundTrip()
    {
        SceneDocument source;
        SceneCameraData camera;

        camera.position = { 4.0f, -2.0f, 7.0f };
        camera.forwardDirection = { 1.0f, 0.0f, 0.0f };
        camera.upDirection = { 0.0f, 0.0f, -1.0f };
        camera.verticalFieldOfViewRadians = glm::radians(65.0f);
        camera.nearClippingPlane = 0.025f;
        camera.farClippingPlane = 750.0f;

        source.camera = camera;

        const std::string text = SerializeSceneDocument(source);
        const Json json = Json::parse(text);

        Require(
            json.at("camera").is_object(),
            "Saved camera must be written as an object.");

        const SceneDocument restored = DeserializeSceneDocument(text);

        Require(
            restored.camera.has_value(),
            "JSON round trip lost the camera.");

        const SceneCameraData& result = *restored.camera;

        Require(
            result.position == camera.position &&
            result.forwardDirection == camera.forwardDirection &&
            result.upDirection == camera.upDirection &&
            result.verticalFieldOfViewRadians ==
            camera.verticalFieldOfViewRadians &&
            result.nearClippingPlane == camera.nearClippingPlane &&
            result.farClippingPlane == camera.farClippingPlane,
            "JSON round trip changed camera data.");

        source.camera.reset();

        const Json withoutCamera =
            Json::parse(SerializeSceneDocument(source));

        Require(
            withoutCamera.contains("camera") &&
            withoutCamera.at("camera").is_null(),
            "Absent camera must be written explicitly as null.");

        Require(
            !DeserializeSceneDocument(withoutCamera.dump()).camera,
            "Null camera should remain absent.");
    }

    void TestVersionOneCameraMigration()
    {
        // Literal legacy fixture: independent of the current serializer.
        constexpr std::string_view legacy = R"json(
    {
        "version": 1,
        "name": "Legacy scene",
        "objects": [],
        "directionalLight": {
            "id": 1,
            "enabled": true,
            "position": [0, 0, 0],
            "color": [1, 1, 1],
            "intensity": 3,
            "directionToLight": [0, 1, 0]
        },
        "pointLights": [],
        "spotLights": []
    }
    )json";

        const SceneDocument restored =
            DeserializeSceneDocument(legacy);

        Require(
            restored.version == CurrentSceneDocumentVersion,
            "Legacy document was not upgraded.");

        Require(
            restored.name == "Legacy scene" &&
            restored.objects.empty() &&
            restored.directionalLight.intensity == 3.0f,
            "Migration changed legacy scene content.");

        Require(
            !restored.camera,
            "Migration invented an authored camera.");

        const Json saved =
            Json::parse(SerializeSceneDocument(restored));

        Require(
            saved.at("version").get<std::uint32_t>() ==
            CurrentSceneDocumentVersion &&
            saved.at("camera").is_null(),
            "Saving a migrated scene did not use the current schema.");
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
        TestCameraRoundTrip();
        TestCameraInputErrors();
        TestVersionOneCameraMigration();
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