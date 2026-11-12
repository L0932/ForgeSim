#include "EditorSession.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"

#include <chrono>
#include <limits>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    using namespace ForgeSim::Sandbox;
    namespace fs = std::filesystem;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    class TemporaryDirectory final
    {
    public:
        TemporaryDirectory()
        {
            const auto stamp =
                std::chrono::steady_clock::now()
                .time_since_epoch().count();

            for (int attempt = 0; attempt < 100; ++attempt)
            {
                const fs::path candidate =
                    fs::temp_directory_path() /
                    ("ForgeSimSceneIO-" + std::to_string(stamp) +
                        "-" + std::to_string(attempt));

                if (fs::create_directory(candidate))
                {
                    m_Path = candidate;
                    return;
                }
            }

            throw std::runtime_error(
                "Could not create the test directory.");
        }

        ~TemporaryDirectory()
        {
            std::error_code ignored;
            fs::remove_all(m_Path, ignored);
        }

        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(
            const TemporaryDirectory&) = delete;

        [[nodiscard]] const fs::path& Path() const noexcept
        {
            return m_Path;
        }

    private:
        fs::path m_Path;
    };

    void WriteTestFile(
        const fs::path& path,
        std::string_view text)
    {
        std::ofstream output{
            path,
            std::ios::out | std::ios::binary | std::ios::trunc
        };

        if (!output)
        {
            throw std::runtime_error(
                "Could not create a test file.");
        }

        output << text;
        output.close();

        Require(
            !output.fail(),
            "Could not finish writing a test file.");
    }

    void ExpectLoadError(
        const fs::path& scenePath,
        const fs::path& projectRoot,
        std::string_view expectedText)
    {
        bool rejected = false;

        try
        {
            const SceneDocument document =
                LoadSceneDocument(scenePath, projectRoot);

            static_cast<void>(document);
        }
        catch (const std::runtime_error& exception)
        {
            rejected = true;

            const std::string message = exception.what();

            Require(
                message.find(scenePath.string()) !=
                std::string::npos,
                "Load error omitted the scene path.");

            Require(
                message.find(expectedText) != std::string::npos,
                "Load error omitted the expected detail.");
        }

        Require(rejected, "Invalid scene file was accepted.");
    }

    void TestModelImport()
    {
        TemporaryDirectory temporary;
        const auto root = temporary.Path();
        fs::create_directories(root / "models");
        WriteTestFile(root / "models/plain.glb", "placeholder");
        WriteTestFile(root / "models/normal.glb", "placeholder");
        SceneDocument document;
        document.objects.push_back(SceneObjectData{.id = 7, .name = "Plain", .transform = {},
            .modelPath = "models/plain.glb"});
        EditorSession active(document, root);
        active.Scene().Objects().front().transform.position.x = -5.0f;
        auto prepared = active.PrepareImportModel(root / "models/normal.glb");
        Require(active.Scene().Objects().size() == 1, "Preparing import changed active scene");
        Require(prepared.Scene().Objects().size() == 2 && prepared.HasUnsavedChanges(),
            "Imported model was not appended as an unsaved change");
        const auto objects = prepared.Scene().Objects();
        Require(objects[0].transform.position.x == -5.0f && objects[1].id == 8 &&
            objects[1].modelPath == "models/normal.glb", "Import changed existing object or reused an ID");
        auto duplicate = prepared.PrepareImportModel(root / "models/normal.glb");
        Require(duplicate.ModelPaths().size() == 2, "Repeated imports should share model resources");
        Require(duplicate.Scene().Objects().back().id == 9, "Repeated model import reused an object ID");
        bool failed = false;
        try { static_cast<void>(active.PrepareImportModel(root / "missing.glb")); }
        catch (const std::exception&) { failed = true; }
        Require(failed && active.Scene().Objects().size() == 1 && active.HasUnsavedChanges(),
            "Failed import lost existing edits");
        prepared.SaveAs(root / "comparison.scene.json");
        const auto reopened = prepared.PrepareOpen(root / "comparison.scene.json", root);
        Require(reopened.Scene().Objects().size() == 2 &&
            reopened.Scene().Objects()[0].modelPath == "models/plain.glb" &&
            reopened.Scene().Objects()[1].modelPath == "models/normal.glb" &&
            !reopened.HasUnsavedChanges(), "Distinct model references did not survive save/reopen");
        TemporaryDirectory outside;
        WriteTestFile(outside.Path() / "outside.glb", "placeholder");
        failed = false;
        try { static_cast<void>(active.PrepareImportModel(outside.Path() / "outside.glb")); }
        catch (const std::invalid_argument&) { failed = true; }
        Require(failed, "Outside-root import should not create an unportable scene reference");
        const auto fresh = active.PrepareNew().PrepareImportModel(outside.Path() / "outside.glb");
        Require(fresh.ProjectRoot() == fs::canonical(outside.Path()) &&
            fresh.Scene().Objects().front().modelPath == "outside.glb" && fresh.HasUnsavedChanges(),
            "First import into a new scene should discover the model's asset root");
    }

    void TestFileLoading()
    {
        TemporaryDirectory temporary;
        const fs::path projectRoot = temporary.Path();

        fs::create_directories(projectRoot / "assets/models");
        fs::create_directories(projectRoot / "scenes");

        const fs::path modelPath =
            projectRoot / "assets/models/Example.glb";

        // This step checks existence, not model-file contents.
        WriteTestFile(modelPath, "model placeholder");

        SceneDocument source;
        source.name = "File loading test";

        source.objects.push_back(
            SceneObjectData{
                .id = 42,
                .name = "Example",
                .transform = Transform{},
                .modelPath = "assets/models/Example.glb"
            });

        const fs::path scenePath =
            projectRoot / "scenes/Example.scene.json";

        WriteTestFile(
            scenePath,
            SerializeSceneDocument(source));

        const SceneDocument loaded =
            LoadSceneDocument(scenePath, projectRoot);

        Require(
            SerializeSceneDocument(loaded) ==
            SerializeSceneDocument(source),
            "Loading from disk changed the document.");

        // Resolution must use projectRoot rather than scenes/.
        ExpectLoadError(
            scenePath,
            projectRoot / "scenes",
            "objects[0].modelPath");

        ExpectLoadError(
            scenePath,
            projectRoot / "missing-root",
            "Project root");

        fs::remove(modelPath);

        ExpectLoadError(
            scenePath,
            projectRoot,
            "objects[0].modelPath");

        // A directory is not a valid model file.
        fs::create_directory(modelPath);

        ExpectLoadError(
            scenePath,
            projectRoot,
            "objects[0].modelPath");

        WriteTestFile(scenePath, "{");

        ExpectLoadError(
            scenePath,
            projectRoot,
            "invalid JSON");

        WriteTestFile(scenePath, "");

        ExpectLoadError(
            scenePath,
            projectRoot,
            "invalid JSON");

        ExpectLoadError(
            projectRoot / "missing.scene.json",
            projectRoot,
            "Could not open");
    }

    void ExpectSaveError(
        const SceneDocument& document,
        const fs::path& scenePath,
        const fs::path& projectRoot,
        std::string_view expectedText)
    {
        bool rejected = false;

        try
        {
            SaveSceneDocument(document, scenePath, projectRoot);
        }
        catch (const std::runtime_error& exception)
        {
            rejected = true;

            const std::string message = exception.what();

            Require(
                message.find(scenePath.string()) !=
                std::string::npos,
                "Save error omitted the scene path.");

            Require(
                message.find(expectedText) != std::string::npos,
                "Save error omitted the expected detail.");
        }

        Require(rejected, "Invalid save operation was accepted.");
    }

    void TestFileSaving()
    {
        TemporaryDirectory temporary;
        const fs::path projectRoot = temporary.Path();

        fs::create_directories(projectRoot / "scenes");
        fs::create_directories(projectRoot / "assets/models");

        WriteTestFile(
            projectRoot / "assets/models/Example.glb",
            "model placeholder");

        SceneDocument source;
        source.name = "Original scene";

        source.objects.push_back(
            SceneObjectData{
                .id = 42,
                .name = "Example",
                .transform = Transform{},
                .modelPath = "assets/models/Example.glb"
            });

        const fs::path scenePath =
            projectRoot / "scenes/Saved.scene.json";

        SaveSceneDocument(source, scenePath, projectRoot);

        Require(
            SerializeSceneDocument(
                LoadSceneDocument(scenePath, projectRoot)) ==
            SerializeSceneDocument(source),
            "Saved scene did not survive a file round trip.");

        // Exercise replacing an existing scene.
        source.name = "Updated scene";
        source.objects[0].transform.position.x = 4.0f;

        SaveSceneDocument(source, scenePath, projectRoot);

        const std::string savedText =
            SerializeSceneDocument(source);

        Require(
            SerializeSceneDocument(
                LoadSceneDocument(scenePath, projectRoot)) ==
            savedText,
            "Saving did not replace the existing scene.");

        // Invalid document must leave the existing scene intact.
        SceneDocument invalid = source;
        invalid.objects[0].transform.scale.x = 0.0f;

        ExpectSaveError(
            invalid,
            scenePath,
            projectRoot,
            "objects[0].transform.scale");

        Require(
            SerializeSceneDocument(
                LoadSceneDocument(scenePath, projectRoot)) ==
            savedText,
            "Rejected save changed the existing scene.");

        // Missing model references must also preserve it.
        invalid = source;
        invalid.objects[0].modelPath = "assets/models/Missing.glb";

        ExpectSaveError(
            invalid,
            scenePath,
            projectRoot,
            "objects[0].modelPath");

        Require(
            SerializeSceneDocument(
                LoadSceneDocument(scenePath, projectRoot)) ==
            savedText,
            "Missing-asset save changed the existing scene.");

        ExpectSaveError(
            source,
            projectRoot / "missing-directory/Scene.json",
            projectRoot,
            "Destination directory");

        // A failed replacement must preserve a destination directory.
        const fs::path directoryTarget =
            projectRoot / "scenes/DirectoryTarget";

        fs::create_directory(directoryTarget);
        WriteTestFile(directoryTarget / "keep.txt", "keep");

        ExpectSaveError(
            source,
            directoryTarget,
            projectRoot,
            "Could not replace");

        Require(
            fs::is_regular_file(directoryTarget / "keep.txt"),
            "Failed replacement damaged the destination directory.");
    }

    void TestEditorSession()
    {
        TemporaryDirectory temporary;
        const fs::path projectRoot = temporary.Path();

        fs::create_directory(projectRoot / "scenes");

        const fs::path firstPath =
            projectRoot / "scenes/First.scene.json";

        const fs::path secondPath =
            projectRoot / "scenes/Second.scene.json";

        // Empty documents require no model files.
        SceneDocument document;
        document.name = "Session test";

        EditorSession session{ document, projectRoot };

        Require(
            !session.ScenePath() &&
            !session.HasUnsavedChanges(),
            "A new session should start without changes.");

        bool rejected = false;

        try
        {
            session.Save();
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }

        Require(
            rejected,
            "Save without a path should require Save As.");

        session.SaveAs(firstPath);

        Require(
            session.ScenePath().has_value() &&
            !session.HasUnsavedChanges(),
            "Successful Save As did not establish a clean baseline.");

        session.Scene().MainDirectionalLight().intensity = 8.0f;

        Require(
            session.HasUnsavedChanges(),
            "Scene edit did not mark the session dirty.");

        session.Scene().MainDirectionalLight().intensity =
            document.directionalLight.intensity;

        Require(
            !session.HasUnsavedChanges(),
            "Restoring saved values did not clear dirty state.");

        session.Scene().MainDirectionalLight().intensity = 8.0f;
        session.Save();

        Require(
            !session.HasUnsavedChanges() &&
            LoadSceneDocument(firstPath, projectRoot)
            .directionalLight.intensity == 8.0f,
            "Save did not persist current scene data.");

        session.Scene().MainDirectionalLight().intensity = 12.0f;

        const auto previousPath = session.ScenePath();

        rejected = false;

        try
        {
            session.SaveAs(
                projectRoot / "missing-directory/Failed.scene.json");
        }
        catch (const std::runtime_error&)
        {
            rejected = true;
        }

        Require(
            rejected &&
            session.ScenePath() == previousPath &&
            session.HasUnsavedChanges(),
            "Failed save changed the path or clean baseline.");

        Require(
            LoadSceneDocument(firstPath, projectRoot)
            .directionalLight.intensity == 8.0f,
            "Failed Save As changed the previous scene file.");

        session.SaveAs(secondPath);

        Require(
            !session.HasUnsavedChanges() &&
            session.ScenePath()->filename() == secondPath.filename(),
            "Save As did not switch the active scene path.");

        const SceneDocument loaded =
            LoadSceneDocument(secondPath, projectRoot);

        EditorSession reopened{
            loaded,
            projectRoot,
            secondPath
        };

        Require(
            !reopened.HasUnsavedChanges(),
            "A loaded session should begin clean.");

        reopened.Scene().MainDirectionalLight().intensity = -1.0f;

        Require(
            reopened.HasUnsavedChanges(),
            "Invalid edits incorrectly appeared saved.");

        EditorSession relativeSession{ document, projectRoot };

        relativeSession.SaveAs("Relative.scene.json");

        const fs::path expectedPath =
            projectRoot / "scenes/Relative.scene.json";

        Require(
            fs::is_regular_file(expectedPath),
            "Relative scene save did not create the expected file.");

        Require(
            relativeSession.ScenePath().has_value() &&
            fs::equivalent(
                *relativeSession.ScenePath(),
                expectedPath),
            "Relative editor saves did not use the scenes directory.");
    }

    void TestPreparedSceneChanges()
    {
        TemporaryDirectory temporary;
        const fs::path projectRoot = temporary.Path();

        SceneDocument original;
        original.name = "Current scene";

        EditorSession active{ original, projectRoot };
        active.SaveAs("Current.scene.json");

        active.Scene().MainDirectionalLight().intensity = 8.0f;

        const auto originalPath = active.ScenePath();

        const std::string originalState =
            SerializeSceneDocument(active.Scene().ToDocument());

        EditorSession fresh = active.PrepareNew();

        Require(
            fresh.Scene().Objects().empty() &&
            fresh.Scene().PointLights().empty() &&
            fresh.Scene().SpotLights().empty() &&
            !fresh.ScenePath() &&
            !fresh.HasUnsavedChanges(),
            "New should prepare an empty, unmodified scene without a path.");

        Require(
            SerializeSceneDocument(active.Scene().ToDocument()) ==
            originalState &&
            active.ScenePath() == originalPath &&
            active.HasUnsavedChanges(),
            "Preparing New modified the active session.");

        SceneDocument other;
        other.name = "Other scene";
        other.directionalLight.intensity = 12.0f;

        EditorSession otherSession{ other, projectRoot };
        otherSession.SaveAs("Other.scene.json");

        EditorSession opened =
            active.PrepareOpen("Other.scene.json");

        Require(
            opened.Scene().Name() == "Other scene" &&
            opened.Scene().MainDirectionalLight().intensity == 12.0f &&
            !opened.HasUnsavedChanges(),
            "Open did not prepare the saved scene correctly.");

        Require(
            SerializeSceneDocument(active.Scene().ToDocument()) ==
            originalState &&
            active.ScenePath() == originalPath &&
            active.HasUnsavedChanges(),
            "Preparing Open modified the active session.");

        WriteTestFile(
            projectRoot / "scenes/Broken.scene.json",
            "{");

        bool rejected = false;

        try
        {
            const EditorSession broken =
                active.PrepareOpen("Broken.scene.json");

            static_cast<void>(broken);
        }
        catch (const std::runtime_error&)
        {
            rejected = true;
        }

        Require(
            rejected &&
            SerializeSceneDocument(active.Scene().ToDocument()) ==
            originalState &&
            active.ScenePath() == originalPath &&
            active.HasUnsavedChanges(),
            "Failed Open damaged the active session.");
    }

    void TestRenderingSettingsPersistence()
    {
        TemporaryDirectory temporary;

        EditorSession session{
            SceneDocument{},
            temporary.Path()
        };

        session.SaveAs("Rendering.scene.json");

        const SceneRenderingSettings original =
            session.Scene().RenderingSettings();

        const auto applySettings = [&session](
            const SceneRenderingSettings& settings)
            {
                Require(
                    !session.Scene().TrySetRenderingSettings(settings),
                    "Valid rendering settings were rejected.");
            };

        // Each individual field must participate in change detection.
        for (int field = 0; field < 4; ++field)
        {
            SceneRenderingSettings edited = original;

            switch (field)
            {
            case 0:
                edited.backgroundColor = { 0.2f, 0.3f, 0.4f };
                break;

            case 1:
                edited.exposure = 2.5f;
                break;

            case 2:
                edited.environmentIntensity = 0.75f;
                break;

            case 3:
                edited.showEnvironmentBackground =
                    !original.showEnvironmentBackground;
                break;
            }

            applySettings(edited);

            Require(
                session.HasUnsavedChanges(),
                "Rendering edit did not mark the session modified.");

            applySettings(original);

            Require(
                !session.HasUnsavedChanges(),
                "Restoring rendering settings did not clear dirty state.");
        }

        SceneRenderingSettings edited = original;
        edited.backgroundColor = { 0.2f, 0.3f, 0.4f };
        edited.exposure = 2.5f;
        edited.environmentIntensity = 0.75f;
        edited.showEnvironmentBackground = true;

        applySettings(edited);
        session.Save();

        Require(
            !session.HasUnsavedChanges(),
            "Saving rendering settings did not clear dirty state.");

        const EditorSession reopened =
            session.PrepareOpen("Rendering.scene.json");

        const SceneRenderingSettings& restored =
            reopened.Scene().RenderingSettings();

        Require(
            restored.backgroundColor == edited.backgroundColor &&
            restored.exposure == edited.exposure &&
            restored.environmentIntensity ==
            edited.environmentIntensity &&
            restored.showEnvironmentBackground ==
            edited.showEnvironmentBackground,
            "Reopening the scene lost rendering settings.");

        Require(
            !reopened.HasUnsavedChanges(),
            "Reopened rendering settings appeared modified.");

        const EditorSession fresh = session.PrepareNew();
        const SceneRenderingSettings defaults;

        const SceneRenderingSettings& freshSettings =
            fresh.Scene().RenderingSettings();

        Require(
            freshSettings.backgroundColor == defaults.backgroundColor &&
            freshSettings.exposure == defaults.exposure &&
            freshSettings.environmentIntensity ==
            defaults.environmentIntensity &&
            freshSettings.showEnvironmentBackground ==
            defaults.showEnvironmentBackground,
            "New scene inherited the previous scene's rendering settings.");
    }
}

namespace
{
    void TestCameraSettingsPersistence()
    {
        TemporaryDirectory temporary;
        const auto root = temporary.Path();
        EditorSession session(SceneDocument{}, root);
        Require(!session.Scene().CameraMovementSpeed(), "Old scenes start with automatic fly speed");
        Require(!session.Scene().TrySetCameraMovementSpeed(0.125f), "Manual speed accepted");
        Require(session.HasUnsavedChanges(), "Changing fly speed marks scene modified");
        SceneCameraData viewpoint;
        viewpoint.position = {1.0f, 2.0f, 3.0f};
        viewpoint.nearClippingPlane = 0.002f;
        viewpoint.farClippingPlane = 750.0f;
        Require(!session.Scene().TrySetSavedCamera(viewpoint), "Captured viewpoint accepted");
        const auto path = root / "camera.scene.json";
        session.SaveAs(path);
        auto reopened = session.PrepareOpen(path);
        Require(reopened.Scene().CameraMovementSpeed() == 0.125f && !reopened.HasUnsavedChanges(),
            "Saved fly speed survives scene-file reopen without dirtying scene");
        Require(reopened.Scene().SavedCamera()->nearClippingPlane == 0.002f &&
            reopened.Scene().SavedCamera()->farClippingPlane == 750.0f &&
            reopened.Scene().SavedCamera()->position == viewpoint.position,
            "Captured pose and clipping survive reopen");
        for (float invalid : {0.0f, -1.0f, 10001.0f,
            std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()})
        {
            Require(reopened.Scene().TrySetCameraMovementSpeed(invalid).has_value() &&
                reopened.Scene().CameraMovementSpeed() == 0.125f,
                "Invalid speed does not alter authored scene setting");
        }
        Require(!reopened.Scene().TrySetCameraMovementSpeed(std::nullopt), "Automatic speed restored");
        Require(reopened.HasUnsavedChanges(), "Restoring automatic speed marks scene modified");
        reopened.Save();
        const auto automatic = LoadSceneDocument(path, root);
        Require(!automatic.cameraMovementSpeed, "Automatic speed survives save and load");
        auto json = nlohmann::json::parse(SerializeSceneDocument(automatic));
        json.erase("cameraMovementSpeed");
        Require(!DeserializeSceneDocument(json.dump()).cameraMovementSpeed,
            "Files written before navigation persistence retain automatic defaults");
        json["cameraMovementSpeed"] = -2.0;
        bool failed = false;
        try { static_cast<void>(DeserializeSceneDocument(json.dump())); }
        catch (const std::invalid_argument&) { failed = true; }
        Require(failed, "Invalid serialized speed rejected");
        Require(!reopened.PrepareNew().Scene().CameraMovementSpeed(),
            "New scene does not inherit previous manual speed");
    }
}

int main()
{
    try
    {
        TestModelImport();
        TestFileLoading();
        TestFileSaving();
        TestEditorSession();
        TestPreparedSceneChanges();
        TestRenderingSettingsPersistence();
        TestCameraSettingsPersistence();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}