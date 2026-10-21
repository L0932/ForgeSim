#include "SandboxApplication.hpp"
#include "SceneDocumentIO.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <ForgeSim/Core/Log.hpp>

namespace
{
    namespace fs = std::filesystem;

    struct CommandLine
    {
        fs::path modelFilename{ "PbrMaterialGallery.glb" };

        bool modelSpecified = false;
        bool help = false;

        std::optional<fs::path> scenePath;
        std::optional<fs::path> projectRoot;
        std::optional<fs::path> saveScenePath;
    };

    [[nodiscard]] CommandLine ParseCommandLine(
        int argumentCount,
        char* arguments[])
    {
        CommandLine result;

        for (int index = 1; index < argumentCount; ++index)
        {
            const std::string_view argument{ arguments[index] };

            if (argument == "--help" || argument == "-h")
            {
                result.help = true;
                continue;
            }

            if (argument == "--scene" ||
                argument == "--project-root" ||
                argument == "--save-scene")
            {
                if (index + 1 >= argumentCount)
                {
                    throw std::invalid_argument(
                        "Missing path after " +
                        std::string{ argument } + ".");
                }

                const fs::path value{ arguments[++index] };

                if (value.empty())
                {
                    throw std::invalid_argument(
                        "Empty path after " +
                        std::string{ argument } + ".");
                }

                std::optional<fs::path>* destination = nullptr;

                if (argument == "--scene")
                {
                    destination = &result.scenePath;
                }
                else if (argument == "--project-root")
                {
                    destination = &result.projectRoot;
                }
                else
                {
                    destination = &result.saveScenePath;
                }

                if (destination->has_value())
                {
                    throw std::invalid_argument(
                        "Repeated option: " +
                        std::string{ argument } + ".");
                }

                *destination = value;
                continue;
            }

            if (argument.starts_with("-"))
            {
                throw std::invalid_argument(
                    "Unknown option: " +
                    std::string{ argument } + ".");
            }

            if (result.modelSpecified)
            {
                throw std::invalid_argument(
                    "Only one positional model path is supported.");
            }

            result.modelFilename = fs::path{ arguments[index] };
            result.modelSpecified = true;
        }

        if (result.scenePath && result.modelSpecified)
        {
            throw std::invalid_argument(
                "Specify either a model path or --scene.");
        }

        return result;
    }

    void PrintHelp()
    {
        std::cout <<
            "ForgeSimSandbox [model.glb] [options]\n"
            "\n"
            "Options:\n"
            "  --scene FILE         Load a scene document.\n"
            "  --project-root DIR   Root for document model references.\n"
            "  --save-scene FILE    Save the scene after normal exit.\n"
            "  --help               Show this help.\n"
            "\n"
            "The default project root contains the copied assets folder.\n"
            "Relative scene-file paths use the working directory.\n"
            "The save destination directory must already exist.\n";
    }

    [[nodiscard]] fs::path DefaultProjectRoot()
    {
        return fs::weakly_canonical(
            fs::absolute(
                fs::path{
                    FORGESIM_SANDBOX_ASSET_DIRECTORY
                }.parent_path()));
    }

    [[nodiscard]] fs::path ResolveSharedModel(
        const ForgeSim::Sandbox::SceneDocument& document,
        const fs::path& projectRoot)
    {
        // The document format supports more than this renderer currently
        // does. Reject unsupported interactive scenes explicitly.
        if (document.objects.empty())
        {
            throw std::invalid_argument(
                "The current Sandbox renderer requires at least "
                "one object when opening a scene document.");
        }

        const std::string& reference =
            document.objects.front().modelPath;

        for (const auto& object : document.objects)
        {
            if (object.modelPath != reference)
            {
                throw std::invalid_argument(
                    "The current Sandbox renderer supports one shared "
                    "model reference per scene. Object '" +
                    object.name + "' references a different model.");
            }
        }

        const std::u8string utf8Reference{
            reference.begin(),
            reference.end()
        };

        return fs::weakly_canonical(
            projectRoot / fs::path{ utf8Reference });
    }
}

int main(int argumentCount, char* arguments[])
{
    try
    {
        const CommandLine commandLine =
            ParseCommandLine(argumentCount, arguments);

        if (commandLine.help)
        {
            PrintHelp();
            return EXIT_SUCCESS;
        }

        ForgeSim::Core::LogInfo(
            "Starting ForgeSim Sandbox Application");

        const fs::path projectRoot =
            fs::weakly_canonical(
                fs::absolute(
                    commandLine.projectRoot.value_or(
                        DefaultProjectRoot())));

        std::optional<ForgeSim::Sandbox::SceneDocument> document;

        fs::path modelFilename = commandLine.modelFilename;

        if (commandLine.scenePath)
        {
            document = ForgeSim::Sandbox::LoadSceneDocument(
                *commandLine.scenePath,
                projectRoot);

            modelFilename = ResolveSharedModel(
                *document,
                projectRoot);
        }

        ForgeSim::Sandbox::SandboxApplication app{
            ForgeSim::Sandbox::SandboxApplicationSpecification{
                .windowSpec = {
                    .width = 1280,
                    .height = 720,
                    .title = "ForgeSim Sandbox",
                    .verticalSync = true
                },
                .rendererSpec = {
                    .sceneFilename = modelFilename
                },
                .initialScene = std::move(document),
                .projectRoot = projectRoot
            }
        };

        app.Run();

        if (commandLine.saveScenePath)
        {
            app.SaveScene(*commandLine.saveScenePath);

            ForgeSim::Core::LogInfo(
                "Saved scene: " +
                commandLine.saveScenePath->string());
        }
    }
    catch (const std::exception& exception)
    {
        ForgeSim::Core::LogError(exception.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}