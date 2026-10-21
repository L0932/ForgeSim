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
}

int main()
{
    try
    {
        TestSceneRoundTrip();
        TestEmptyScene();
        TestInvalidDocument();
        TestIdExhaustion();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}