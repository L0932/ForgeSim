#include "EditorSession.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"

#include <chrono>
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

        Require(
            relativeSession.ScenePath().value() ==
            (projectRoot / "scenes/Relative.scene.json")
            .lexically_normal(),
            "Relative editor saves did not use the scenes directory.");

        Require(
            fs::is_regular_file(
                projectRoot / "scenes/Relative.scene.json"),
            "Relative scene save did not create the expected file.");
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
}

int main()
{
    try
    {
        TestFileLoading();
        TestFileSaving();
        TestEditorSession();
        TestPreparedSceneChanges();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}