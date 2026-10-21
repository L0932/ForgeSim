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
}

int main()
{
    try
    {
        TestFileLoading();
        TestFileSaving();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}