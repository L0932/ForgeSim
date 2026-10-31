#include "SandboxScene.hpp"
#include "SceneDocument.hpp"
#include "SceneSerializer.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

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
        document.name = "Chapel";

        document.objects.push_back(
            SceneObjectData{
                .id = 42,
                .name = "Icon panel",
                .transform = Transform{
                    .position = { 1.0f, 2.0f, 3.0f },
                    .rotationRadians = { 0.25f, 0.5f, 0.75f },
                    .scale = { -1.0f, 2.0f, 0.5f }
                },
                .modelPath = "assets/models/Icon.glb"
            });

        document.directionalLight.id = 10;
        document.directionalLight.position = {
            4.0f, 5.0f, 6.0f
        };

        document.pointLights.push_back(
            PointLight{
                .id = 20,
                .enabled = false,
                .intensity = 8.0f
            });

        document.spotLights.push_back(
            SpotLight{
                .id = 30,
                .emissionDirection = { 0.0f, -5.0f, 0.0f },
                .intensity = 12.0f
            });

        return document;
    }

    void TestSceneRoundTrip()
    {
        const SceneDocument source = MakeDocument();
        SandboxScene scene{ source };

        Require(
            scene.Name() == source.name,
            "Scene name was not restored.");

        Require(
            scene.FindObject(42) != nullptr &&
            scene.FindObject(1) == nullptr,
            "Construction did not preserve the document's objects.");

        Require(
            SerializeSceneDocument(scene.ToDocument()) ==
            SerializeSceneDocument(source),
            "Runtime scene conversion changed document data.");

        SandboxObject* object = scene.FindObject(42);
        object->transform.position.x = 9.0f;
        object->name = "Updated icon";

        scene.PointLights()[0].intensity = 15.0f;

        const SceneDocument captured = scene.ToDocument();

        Require(
            captured.objects[0].transform.position.x == 9.0f &&
            captured.objects[0].name == "Updated icon" &&
            captured.pointLights[0].intensity == 15.0f,
            "Capture did not include current runtime edits.");

        Require(
            captured.objects[0].modelPath ==
            source.objects[0].modelPath,
            "Capture lost the model reference.");

        Require(
            scene.AddPointLight() == 31 &&
            scene.AddSpotLight() == 32,
            "New lights collided with restored IDs.");
    }

    void TestEmptyScene()
    {
        const SceneDocument document;
        const SandboxScene scene{ document };

        Require(
            scene.Objects().empty(),
            "Loading an empty scene inserted a demonstration object.");

        Require(
            SerializeSceneDocument(scene.ToDocument()) ==
            SerializeSceneDocument(document),
            "Empty scene did not survive conversion.");
    }

    void TestInvalidDocument()
    {
        SceneDocument document = MakeDocument();
        document.pointLights[0].id =
            document.directionalLight.id;

        bool rejected = false;

        try
        {
            const SandboxScene scene{ document };
            static_cast<void>(scene);
        }
        catch (const std::invalid_argument& exception)
        {
            rejected = true;

            Require(
                std::string{ exception.what() }.find(
                    "pointLights[0].id") != std::string::npos,
                "Construction error omitted the invalid property.");
        }

        Require(rejected, "Invalid scene document was accepted.");
    }

    void TestIdExhaustion()
    {
        constexpr SceneLightId maximum =
            std::numeric_limits<SceneLightId>::max();

        SceneDocument document;
        document.directionalLight.id = maximum - 1;

        SandboxScene scene{ document };

        Require(
            scene.AddPointLight() == maximum,
            "Last available light ID was not allocated.");

        bool rejected = false;

        try
        {
            static_cast<void>(scene.AddSpotLight());
        }
        catch (const std::overflow_error&)
        {
            rejected = true;
        }

        Require(rejected, "Light ID allocation wrapped around.");
        Require(
            scene.SpotLights().empty(),
            "Failed allocation inserted a light.");

        // An exhausted scene must also survive save/load conversion.
        SandboxScene restored{ scene.ToDocument() };

        rejected = false;

        try
        {
            static_cast<void>(restored.AddPointLight());
        }
        catch (const std::overflow_error&)
        {
            rejected = true;
        }

        Require(
            rejected,
            "Restoring an exhausted scene reset its ID sequence.");
    }

    void TestSavedCameraPreservation()
    {
        SceneDocument source = MakeDocument();

        const SandboxScene withoutCamera{ source };

        Require(
            !withoutCamera.ToDocument().camera,
            "Conversion invented a saved camera.");

        SceneCameraData camera;
        camera.position = { 4.0f, 2.0f, -3.0f };
        camera.forwardDirection = { 0.0f, 0.0f, 1.0f };
        camera.upDirection = { 1.0f, 0.0f, 0.0f };
        camera.verticalFieldOfViewRadians = glm::radians(60.0f);
        camera.nearClippingPlane = 0.025f;
        camera.farClippingPlane = 250.0f;

        source.camera = camera;

        SandboxScene scene{ source };

        // Ordinary scene edits must not discard the authored viewpoint.
        scene.FindObject(42)->transform.position.x = 8.0f;

        const SceneDocument captured = scene.ToDocument();

        Require(
            captured.camera.has_value(),
            "Conversion lost the saved camera.");

        const SceneCameraData& restored = *captured.camera;

        Require(
            restored.position == camera.position &&
            restored.forwardDirection == camera.forwardDirection &&
            restored.upDirection == camera.upDirection &&
            restored.verticalFieldOfViewRadians ==
            camera.verticalFieldOfViewRadians &&
            restored.nearClippingPlane == camera.nearClippingPlane &&
            restored.farClippingPlane == camera.farClippingPlane,
            "Conversion changed saved camera data.");

        // Reconstruction must use document-level camera validation.
        source.camera->farClippingPlane =
            source.camera->nearClippingPlane;

        bool rejected = false;

        try
        {
            const SandboxScene invalidScene{ source };
            static_cast<void>(invalidScene);
        }
        catch (const std::invalid_argument& exception)
        {
            rejected = true;

            Require(
                std::string{ exception.what() }.find(
                    "camera.farClippingPlane") != std::string::npos,
                "Construction error omitted the camera property.");
        }

        Require(rejected, "Construction accepted an invalid saved camera.");
    }

    void TestSavedCameraEditing()
    {
        SandboxScene scene{ MakeDocument() };

        Require(
            !scene.SavedCamera().has_value(),
            "Scene unexpectedly started with a saved camera.");

        const std::string original =
            SerializeSceneDocument(scene.ToDocument());

        SceneCameraData camera;
        camera.position = { 7.0f, 3.0f, -2.0f };
        camera.forwardDirection = { 1.0f, 0.0f, 0.0f };
        camera.upDirection = { 0.0f, 0.0f, 1.0f };
        camera.verticalFieldOfViewRadians = glm::radians(60.0f);
        camera.nearClippingPlane = 0.1f;
        camera.farClippingPlane = 500.0f;

        Require(
            !scene.TrySetSavedCamera(camera).has_value(),
            "Valid camera was rejected.");

        Require(
            scene.SavedCamera().has_value(),
            "Saving a camera did not retain it.");

        const SceneCameraData& saved = *scene.SavedCamera();

        Require(
            saved.position == camera.position &&
            saved.forwardDirection == camera.forwardDirection &&
            saved.upDirection == camera.upDirection &&
            saved.verticalFieldOfViewRadians ==
            camera.verticalFieldOfViewRadians &&
            saved.nearClippingPlane == camera.nearClippingPlane &&
            saved.farClippingPlane == camera.farClippingPlane,
            "Saving a camera changed its values.");

        const std::string captured =
            SerializeSceneDocument(scene.ToDocument());

        Require(
            captured != original,
            "Saving a camera did not change the scene document.");

        SceneCameraData invalid = camera;
        invalid.nearClippingPlane = 0.0f;

        const auto error = scene.TrySetSavedCamera(invalid);

        Require(
            error.has_value(),
            "Invalid camera was accepted.");

        Require(
            error->property == "nearClippingPlane",
            "Camera error identified the wrong property.");

        Require(
            SerializeSceneDocument(scene.ToDocument()) == captured,
            "Rejected camera edit changed the scene document.");

        scene.ClearSavedCamera();

        Require(
            !scene.SavedCamera().has_value(),
            "Clearing the camera retained a saved viewpoint.");

        Require(
            SerializeSceneDocument(scene.ToDocument()) == original,
            "Clearing the camera changed unrelated scene data.");

        scene.ClearSavedCamera();

        Require(
            SerializeSceneDocument(scene.ToDocument()) == original,
            "Clearing an absent camera changed the scene document.");
    }
}

int main()
{
    try
    {
        TestSavedCameraPreservation();
        TestSceneRoundTrip();
        TestEmptyScene();
        TestInvalidDocument();
        TestIdExhaustion();
        TestSavedCameraEditing();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}