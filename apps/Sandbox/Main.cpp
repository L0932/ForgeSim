#include "SandboxApplication.hpp"
#include "SandboxPaths.hpp"
#include "SceneDocumentIO.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <ForgeSim/Build/Version.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Platform/ApplicationPaths.hpp>

namespace
{
    namespace fs = std::filesystem;

    struct CommandLine
    {
        fs::path modelFilename{ "PbrMaterialGallery.glb" };

        bool modelSpecified = false;
        bool help = false;
        bool allowTextureFallbacks = false;
        bool loadingDiagnostics = false;
        bool parallelImageDecode = true;
        bool imageDecodeModeSpecified = false;

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

            if (argument == "--loading-diagnostics")
            {
                if (result.loadingDiagnostics)
                    throw std::invalid_argument("Repeated option: --loading-diagnostics.");
                result.loadingDiagnostics = true;
                continue;
            }

            if (argument == "--parallel-image-decode" || argument == "--serial-image-decode")
            {
                if (result.imageDecodeModeSpecified)
                    throw std::invalid_argument("Specify only one image decode mode.");
                result.imageDecodeModeSpecified = true;
                result.parallelImageDecode = argument == "--parallel-image-decode";
                continue;
            }

            if (argument == "--allow-texture-fallbacks")
            {
                if (result.allowTextureFallbacks)
                {
                    throw std::invalid_argument(
                        "Repeated option: --allow-texture-fallbacks.");
                }

                result.allowTextureFallbacks = true;
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
            "  --allow-texture-fallbacks\n"
            "                      Skip textures with missing or unusable UV sets.\n"
            "                      Applies to all model loads in this session.\n"
            "  --loading-diagnostics   Log scene loading progress and timings.\n"
            "  --parallel-image-decode Decode glTF images concurrently (default).\n"
            "  --serial-image-decode   Decode glTF images one at a time.\n"
            "  --help               Show this help.\n"
            "\n"
            "The default project root contains the copied assets folder.\n"
            "Relative scene-file paths use the working directory.\n"
            "The save destination directory must already exist.\n";
    }

    [[nodiscard]] fs::path DefaultProjectRoot()
    {
        return ForgeSim::Platform::ExecutableDirectory();
    }

    [[nodiscard]] fs::path ResolveSharedModel(
        const ForgeSim::Sandbox::SceneDocument& document,
        const fs::path& projectRoot)
    {
        // The document format supports more than this renderer currently
        // does. Reject unsupported interactive scenes explicitly.
        if (document.objects.empty())
        {
            // No object is drawn. Keep the default model resource
            // available for the current renderer.
            return fs::path{ "PbrMaterialGallery.glb" };
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
    const auto startupStart = std::chrono::steady_clock::now();
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
            std::string{ "Starting ForgeSim Sandbox " } +
            ForgeSim::Build::Version);

        const fs::path executableDirectory =
            ForgeSim::Platform::ExecutableDirectory();

        ForgeSim::Core::LogInfo(
            "Executable directory: " +
            executableDirectory.string());

        ForgeSim::Core::LogInfo(
            "Bundled assets: " +
            ForgeSim::Sandbox::BundledAssetDirectory().string());

        ForgeSim::Core::LogInfo(
            "Shader directory: " +
            ForgeSim::Sandbox::ShaderDirectory().string());

        if (commandLine.allowTextureFallbacks)
        {
            ForgeSim::Core::LogWarning(
                "glTF texture fallbacks enabled for this session. "
                "Skipped textures will be reported; material factors are retained.");
        }

        const fs::path projectRoot =
            fs::weakly_canonical(
                fs::absolute(
                    commandLine.projectRoot.value_or(
                        DefaultProjectRoot())));

        std::optional<ForgeSim::Sandbox::SceneDocument> document;

        fs::path modelFilename = commandLine.modelFilename;

        const ForgeSim::Platform::WindowSpecification windowSpec{
            .width = 1280,
            .height = 720,
            .title = "ForgeSim Sandbox",
            .verticalSync = true
        };

        if (commandLine.scenePath)
        {
            try
            {
                const auto sceneStart = std::chrono::steady_clock::now();
                document = ForgeSim::Sandbox::LoadSceneDocument(
                    *commandLine.scenePath,
                    projectRoot);

                const auto sceneMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - sceneStart).count();
                if (commandLine.loadingDiagnostics)
                    ForgeSim::Core::LogInfo("Scene document: " + std::to_string(sceneMs) +
                    " ms (" + commandLine.scenePath->string() + ")");

                modelFilename = ResolveSharedModel(
                    *document,
                    projectRoot);
            }
            catch (const std::exception& exception)
            {
                ForgeSim::Core::LogError(exception.what());
                ForgeSim::Sandbox::SandboxApplication::ShowStartupError(windowSpec, exception.what());
                return EXIT_FAILURE;
            }
        }

        ForgeSim::Sandbox::SandboxApplication app{
            ForgeSim::Sandbox::SandboxApplicationSpecification{
                .windowSpec = windowSpec,
                .rendererSpec = {
                    .sceneFilename = modelFilename,
                    .texturePolicy = commandLine.allowTextureFallbacks
                        ? ForgeSim::Assets::GltfTexturePolicy::SkipUnusableTextures
                        : ForgeSim::Assets::GltfTexturePolicy::Strict,
                    .loadingDiagnostics = commandLine.loadingDiagnostics,
                    .parallelImageDecode = commandLine.parallelImageDecode
                },
                .initialScene = std::move(document),
                .projectRoot = projectRoot,
                .scenePath = commandLine.scenePath
            }
        };

        app.Run(commandLine.loadingDiagnostics ? std::optional{startupStart} : std::nullopt);

        if (commandLine.saveScenePath && app.ShouldSaveOnExit())
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
