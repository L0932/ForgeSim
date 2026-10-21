#include "SceneDocumentValidation.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace
{
    using namespace ForgeSim::Sandbox;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    [[nodiscard]] SceneDocument MakeDocument()
    {
        SceneDocument document;
        document.name = "Validation scene";

        document.objects.push_back(
            SceneObjectData{
                .id = 1,
                .name = "Example",
                .transform = Transform{},
                .modelPath = "assets/models/Example.glb"
            });

        document.pointLights.push_back(
            PointLight{
                .id = 2
            });

        document.spotLights.push_back(
            SpotLight{
                .id = 3
            });

        return document;
    }

    void ExpectError(
        const SceneDocument& document,
        std::string_view property)
    {
        const auto error = ValidateSceneDocument(document);

        Require(error.has_value(), "Invalid document was accepted.");
        Require(
            error->property == property,
            "Validation reported the wrong property.");
        Require(
            !error->message.empty(),
            "Validation must provide an error message.");
    }

    void TestValidDocuments()
    {
        Require(
            !ValidateSceneDocument(SceneDocument{}),
            "An empty scene with the default light should be valid.");

        SceneDocument document = MakeDocument();

        Require(
            !ValidateSceneDocument(document),
            "Valid document was rejected.");

        // Object ID 1 and directional-light ID 1 are independent.
        document.objects[0].transform.scale = {
            -1.0f, 2.0f, 0.5f
        };

        document.spotLights[0].emissionDirection = {
            0.0f, -5.0f, 0.0f
        };

        Require(
            !ValidateSceneDocument(document),
            "Negative scale or non-unit direction was rejected.");

        Require(
            document.objects[0].transform.scale.x == -1.0f &&
            document.spotLights[0].emissionDirection.y == -5.0f,
            "Validation modified scene data.");
    }

    void TestVersionAndNames()
    {
        SceneDocument document = MakeDocument();

        document.version = CurrentSceneDocumentVersion + 1;
        ExpectError(document, "version");

        document = MakeDocument();
        document.name.clear();
        ExpectError(document, "name");

        document = MakeDocument();
        document.objects[0].name.clear();
        ExpectError(document, "objects[0].name");
    }

    void TestIds()
    {
        SceneDocument document = MakeDocument();

        document.objects[0].id = 0;
        ExpectError(document, "objects[0].id");

        document = MakeDocument();
        const SceneObjectData duplicate = document.objects[0];
        document.objects.push_back(duplicate);
        ExpectError(document, "objects[1].id");

        document = MakeDocument();
        document.directionalLight.id = 0;
        ExpectError(document, "directionalLight.id");

        document = MakeDocument();
        document.pointLights[0].id = 0;
        ExpectError(document, "pointLights[0].id");

        document = MakeDocument();
        document.spotLights[0].id = 0;
        ExpectError(document, "spotLights[0].id");

        document = MakeDocument();
        document.pointLights[0].id =
            document.directionalLight.id;
        ExpectError(document, "pointLights[0].id");

        document = MakeDocument();
        document.spotLights[0].id = document.pointLights[0].id;
        ExpectError(document, "spotLights[0].id");
    }

    void TestTransforms()
    {
        const float nan =
            std::numeric_limits<float>::quiet_NaN();

        const float infinity =
            std::numeric_limits<float>::infinity();

        SceneDocument document = MakeDocument();
        document.objects[0].transform.position.x = nan;
        ExpectError(document, "objects[0].transform.position");

        document = MakeDocument();
        document.objects[0].transform.rotationRadians.y = infinity;
        ExpectError(document, "objects[0].transform.rotationRadians");

        document = MakeDocument();
        document.objects[0].transform.scale.z = 0.0f;
        ExpectError(document, "objects[0].transform.scale");

        document = MakeDocument();
        document.objects[0].transform.scale.x = nan;
        ExpectError(document, "objects[0].transform.scale");
    }

    void TestModelPaths()
    {
        constexpr const char* invalidPaths[]{
            "",
            "/assets/model.glb",
            "C:/assets/model.glb",
            "C:model.glb",
            R"(assets\model.glb)",
            "../model.glb",
            "assets/../model.glb",
            "./assets/model.glb",
            "assets//model.glb",
            "assets/model.glb/",
            "assets/\nmodel.glb"
        };

        for (const char* path : invalidPaths)
        {
            SceneDocument document = MakeDocument();
            document.objects[0].modelPath = path;

            ExpectError(document, "objects[0].modelPath");
        }

        SceneDocument document = MakeDocument();
        document.objects[0].modelPath =
            "assets/models/Church Interior.glb";

        Require(
            !ValidateSceneDocument(document),
            "Spaces in model paths should be supported.");

        document.objects[0].modelPath =
            std::string{ "assets/model" } + '\0' + ".glb";

        ExpectError(document, "objects[0].modelPath");
    }

    void TestLightValidationIntegration()
    {
        SceneDocument document = MakeDocument();

        document.directionalLight.directionToLight = {
            0.0f, 0.0f, 0.0f
        };

        ExpectError(
            document,
            "directionalLight.directionToLight");

        document = MakeDocument();
        document.pointLights[0].enabled = false;
        document.pointLights[0].range = -1.0f;

        ExpectError(document, "pointLights[0].range");

        document = MakeDocument();
        document.spotLights[0].outerConeAngleRadians =
            document.spotLights[0].innerConeAngleRadians;

        ExpectError(
            document,
            "spotLights[0].outerConeAngleRadians");
    }
}

int main()
{
    try
    {
        TestValidDocuments();
        TestVersionAndNames();
        TestIds();
        TestTransforms();
        TestModelPaths();
        TestLightValidationIntegration();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}