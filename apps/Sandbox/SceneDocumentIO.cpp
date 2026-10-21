#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>

namespace ForgeSim::Sandbox
{
    namespace
    {
        namespace fs = std::filesystem;

        [[nodiscard]] fs::path ModelPathFromUtf8(
            const std::string& value)
        {
            // Document paths are UTF-8, including on Windows.
            return fs::path{
                std::u8string{ value.begin(), value.end() }
            };
        }

        void ValidateModelFiles(
            const SceneDocument& document,
            const fs::path& projectRoot)
        {
            std::error_code error;

            if (!fs::is_directory(projectRoot, error) || error)
            {
                throw std::runtime_error(
                    "Project root is not an accessible directory: " +
                    projectRoot.string());
            }

            for (std::size_t index = 0;
                index < document.objects.size();
                ++index)
            {
                const SceneObjectData& object =
                    document.objects[index];

                const fs::path resolvedPath =
                    projectRoot / ModelPathFromUtf8(object.modelPath);

                error.clear();

                if (!fs::is_regular_file(resolvedPath, error) || error)
                {
                    throw std::runtime_error(
                        "objects[" + std::to_string(index) +
                        "].modelPath: Model is not an accessible "
                        "regular file: " + resolvedPath.string());
                }
            }
        }

        class PendingSceneFile final
        {
        public:
            explicit PendingSceneFile(const fs::path& parent)
            {
                static std::atomic<std::uint64_t> sequence{ 0 };

                const auto stamp =
                    std::chrono::steady_clock::now()
                    .time_since_epoch().count();

                for (int attempt = 0; attempt < 100; ++attempt)
                {
                    const auto number = sequence.fetch_add(
                        1,
                        std::memory_order_relaxed);

                    const fs::path candidate =
                        parent /
                        (".ForgeSimSceneSave-" +
                            std::to_string(stamp) + "-" +
                            std::to_string(number));

                    std::error_code error;

                    // Creating the directory reserves a unique location.
                    if (fs::create_directory(candidate, error))
                    {
                        m_Directory = candidate;
                        return;
                    }

                    if (error &&
                        error != std::errc::file_exists)
                    {
                        throw std::runtime_error(
                            "Could not create temporary save directory: " +
                            error.message());
                    }
                }

                throw std::runtime_error(
                    "Could not reserve a temporary save location.");
            }

            ~PendingSceneFile()
            {
                std::error_code ignored;
                fs::remove_all(m_Directory, ignored);
            }

            PendingSceneFile(const PendingSceneFile&) = delete;
            PendingSceneFile& operator=(
                const PendingSceneFile&) = delete;

            [[nodiscard]] fs::path Path() const
            {
                return m_Directory / "document.tmp";
            }

        private:
            fs::path m_Directory;
        };
    }

    SceneDocument LoadSceneDocument(
        const std::filesystem::path& scenePath,
        const std::filesystem::path& projectRoot)
    {
        try
        {
            std::ifstream input{
                scenePath,
                std::ios::in | std::ios::binary
            };

            if (!input)
            {
                throw std::runtime_error(
                    "Could not open the scene file for reading.");
            }

            const std::string text{
                std::istreambuf_iterator<char>{ input },
                std::istreambuf_iterator<char>{}
            };

            if (input.bad())
            {
                throw std::runtime_error(
                    "An error occurred while reading the scene file.");
            }

            SceneDocument document =
                DeserializeSceneDocument(text);

            ValidateModelFiles(document, projectRoot);

            return document;
        }
        catch (const std::exception& exception)
        {
            throw std::runtime_error(
                "Could not load scene '" + scenePath.string() +
                "': " + exception.what());
        }
    }

    void SaveSceneDocument(
        const SceneDocument& document,
        const std::filesystem::path& scenePath,
        const std::filesystem::path& projectRoot)
    {
        try
        {
            // Complete validation before opening any output file.
            const std::string text =
                SerializeSceneDocument(document);

            ValidateModelFiles(document, projectRoot);

            if (scenePath.empty() || scenePath.filename().empty())
            {
                throw std::runtime_error(
                    "Destination must name a scene file.");
            }

            const std::filesystem::path destination =
                std::filesystem::absolute(scenePath);

            const std::filesystem::path parent =
                destination.parent_path();

            std::error_code error;

            if (!std::filesystem::is_directory(parent, error) ||
                error)
            {
                throw std::runtime_error(
                    "Destination directory does not exist "
                    "or is inaccessible.");
            }

            PendingSceneFile pending{ parent };
            const std::filesystem::path temporaryPath =
                pending.Path();

            std::ofstream output{
                temporaryPath,
                std::ios::out | std::ios::binary | std::ios::trunc
            };

            if (!output)
            {
                throw std::runtime_error(
                    "Could not open the temporary scene file.");
            }

            output << text;
            output.close();

            if (output.fail())
            {
                throw std::runtime_error(
                    "Could not finish writing the temporary scene file.");
            }

            error.clear();

            std::filesystem::rename(
                temporaryPath,
                destination,
                error);

            if (error)
            {
                throw std::runtime_error(
                    "Could not replace the destination scene file: " +
                    error.message());
            }
        }
        catch (const std::exception& exception)
        {
            throw std::runtime_error(
                "Could not save scene '" + scenePath.string() +
                "': " + exception.what());
        }
    }
}