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

    void RequireRenderingEqual(
        const SceneRenderingSettings& actual,
        const SceneRenderingSettings& expected)
    {
        Require(
            actual.backgroundColor == expected.backgroundColor &&
            actual.exposure == expected.exposure &&
            actual.environmentIntensity == expected.environmentIntensity &&
            actual.showEnvironmentBackground ==
            expected.showEnvironmentBackground,
            "Rendering settings changed during serialization.");
    }

    void TestRenderingRoundTrip()
    {
        SceneDocument source;
        source.rendering.backgroundColor = { 0.15f, 0.25f, 0.35f };
        source.rendering.exposure = 2.5f;
        source.rendering.environmentIntensity = 0.75f;
        source.rendering.showEnvironmentBackground = true;

        const std::string text = SerializeSceneDocument(source);
        const Json json = Json::parse(text);
        const Json& rendering = json.at("rendering");

        Require(
            rendering.at("backgroundColor") ==
            Json::array({ 0.15f, 0.25f, 0.35f }) &&
            rendering.at("exposure") == 2.5f &&
            rendering.at("environmentIntensity") == 0.75f &&
            rendering.at("showEnvironmentBackground") == true,
            "Serialized rendering fields are incorrect.");

        const SceneDocument restored = DeserializeSceneDocument(text);

        RequireRenderingEqual(restored.rendering, source.rendering);
    }

    void TestMissingRenderingUsesDefaults()
    {
        Json json = Json::parse(
            SerializeSceneDocument(SceneDocument{}));

        // Simulate a version 2 scene written before rendering persistence.
        json["version"] = 2;
        json.erase("rendering");

        const SceneDocument restored =
            DeserializeSceneDocument(json.dump());

        RequireRenderingEqual(
            restored.rendering,
            SceneRenderingSettings{});

        const Json saved = Json::parse(
            SerializeSceneDocument(restored));

        Require(
            saved.contains("rendering"),
            "Resaving an older scene omitted rendering settings.");
    }

    void TestMalformedRendering()
    {
        const Json valid = Json::parse(
            SerializeSceneDocument(SceneDocument{}));

        for (const char* field : {
            "backgroundColor",
            "exposure",
            "environmentIntensity",
            "showEnvironmentBackground" })
        {
            Json candidate = valid;
            candidate["rendering"].erase(field);

            ExpectReadError(
                candidate.dump(),
                std::string{ "rendering." } + field);
        }

        Json candidate = valid;
        candidate["rendering"] = nullptr;
        ExpectReadError(candidate.dump(), "rendering");

        candidate = valid;
        candidate["rendering"]["backgroundColor"] =
            Json::array({ 0.1f, 0.2f });
        ExpectReadError(
            candidate.dump(),
            "rendering.backgroundColor");

        candidate = valid;
        candidate["rendering"]["backgroundColor"][0] = -0.1f;
        ExpectReadError(
            candidate.dump(),
            "rendering.backgroundColor");

        candidate = valid;
        candidate["rendering"]["exposure"] = "bright";
        ExpectReadError(candidate.dump(), "rendering.exposure");

        candidate = valid;
        candidate["rendering"]["exposure"] = 0.0f;
        ExpectReadError(candidate.dump(), "rendering.exposure");

        candidate = valid;
        candidate["rendering"]["environmentIntensity"] = -1.0f;
        ExpectReadError(
            candidate.dump(),
            "rendering.environmentIntensity");

        candidate = valid;
        candidate["rendering"]["showEnvironmentBackground"] = 1;
        ExpectReadError(
            candidate.dump(),
            "rendering.showEnvironmentBackground");
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
    void TestPivotPersistence()
    {
        SceneDocument document;
        document.objects.push_back(SceneObjectData{
            .id = 1, .name = "Offset model", .transform = Transform{},
            .modelPath = "assets/models/OffsetHierarchy.gltf"});
        document.objects[0].transform.pivot = {5.0f, 2.0f, -1.0f};
        const auto text = SerializeSceneDocument(document);
        const auto restored = DeserializeSceneDocument(text);
        Require(restored.objects[0].transform.pivot == document.objects[0].transform.pivot,
            "Object pivot did not survive scene serialization.");
        auto legacy = Json::parse(text);
        legacy["version"] = 2;
        legacy["objects"][0]["transform"].erase("pivot");
        Require(DeserializeSceneDocument(legacy.dump()).objects[0].transform.pivot == glm::vec3{0.0f},
            "Version 2 scene did not retain authored-origin behavior.");
        auto invalid = Json::parse(text);
        invalid["objects"][0]["transform"]["pivot"] = Json::array({0, "invalid", 0});
        ExpectReadError(invalid.dump(), "objects[0].transform.pivot");
        document.objects[0].transform.pivot.x = std::numeric_limits<float>::infinity();
        bool rejected = false;
        try { static_cast<void>(SerializeSceneDocument(document)); }
        catch (const std::invalid_argument&) { rejected = true; }
        Require(rejected, "Non-finite object pivot was serialized.");
    }

    void TestEditorVisibilityPersistence()
    {
        SceneDocument document;
        constexpr std::uint64_t objectId = 9007199254740993ULL;
        document.objects.push_back(SceneObjectData{.id = objectId, .name = "Model",
            .transform = {}, .modelPath = "assets/model.glb"});
        document.pointLights.push_back(PointLight{.id = 2, .enabled = false});
        document.spotLights.push_back(SpotLight{.id = 3});
        document.hiddenObjectIds = {objectId};
        document.hiddenLightIds = {1,2,3};
        const auto text = SerializeSceneDocument(document);
        const auto restored = DeserializeSceneDocument(text);
        Require(restored.hiddenObjectIds == document.hiddenObjectIds &&
            restored.hiddenLightIds == document.hiddenLightIds,
            "Editor visibility did not survive serialization.");
        Require(!restored.pointLights.front().enabled && restored.spotLights.front().enabled,
            "Visibility changed authored light enabled settings.");

        for (int version = 1; version <= 3; ++version)
        {
            auto legacy = Json::parse(text);
            legacy["version"] = version;
            legacy.erase("editorVisibility");
            const auto migrated = DeserializeSceneDocument(legacy.dump());
            Require(migrated.version == CurrentSceneDocumentVersion &&
                migrated.hiddenObjectIds.empty() && migrated.hiddenLightIds.empty(),
                "A legacy scene did not default to visible objects and lights.");
        }
        auto invalid = Json::parse(text);
        invalid["editorVisibility"]["hiddenObjectIds"] = {42};
        ExpectReadError(invalid.dump(), "editorVisibility.hiddenObjectIds[0]");
        invalid = Json::parse(text);
        invalid["editorVisibility"]["hiddenLightIds"] = {1,1};
        ExpectReadError(invalid.dump(), "editorVisibility.hiddenLightIds[1]");
        invalid["editorVisibility"]["hiddenLightIds"] = {true};
        ExpectReadError(invalid.dump(), "editorVisibility.hiddenLightIds[0]");
        invalid["editorVisibility"]["hiddenLightIds"] = "invalid";
        ExpectReadError(invalid.dump(), "editorVisibility.hiddenLightIds");
        invalid["editorVisibility"] = Json::array();
        ExpectReadError(invalid.dump(), "editorVisibility");
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
        TestRenderingRoundTrip();
        TestMissingRenderingUsesDefaults();
        TestMalformedRendering();
        TestPivotPersistence();
        TestEditorVisibilityPersistence();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}