#include "EditorSession.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"

#include <stdexcept>
#include <utility>

namespace ForgeSim::Sandbox
{
    EditorSession::EditorSession(
        const SceneDocument& document,
        const std::filesystem::path& projectRoot,
        std::optional<std::filesystem::path> scenePath)
        : m_Scene(document)
        , m_ProjectRoot(
            std::filesystem::weakly_canonical(
                std::filesystem::absolute(projectRoot)))
    {
        if (scenePath)
        {
            if (scenePath->empty() ||
                scenePath->filename().empty())
            {
                throw std::invalid_argument(
                    "Scene path must name a file.");
            }

            m_ScenePath =
                std::filesystem::absolute(*scenePath)
                .lexically_normal();
        }

        ResetChangeBaseline();
    }

    SandboxScene& EditorSession::Scene() noexcept
    {
        return m_Scene;
    }

    const SandboxScene& EditorSession::Scene() const noexcept
    {
        return m_Scene;
    }

    const std::optional<std::filesystem::path>&
        EditorSession::ScenePath() const noexcept
    {
        return m_ScenePath;
    }

    const std::filesystem::path&
        EditorSession::ProjectRoot() const noexcept
    {
        return m_ProjectRoot;
    }

    bool EditorSession::HasUnsavedChanges() const
    {
        try
        {
            return SerializeSceneDocument(m_Scene.ToDocument()) !=
                m_ChangeBaseline;
        }
        catch (const std::invalid_argument&)
        {
            // Invalid edits must still count as changes.
            return true;
        }
    }

    void EditorSession::ResetChangeBaseline()
    {
        m_ChangeBaseline =
            SerializeSceneDocument(m_Scene.ToDocument());
    }

    EditorSession EditorSession::PrepareNew() const
    {
        return EditorSession{
            SceneDocument{},
            m_ProjectRoot
        };
    }

    EditorSession EditorSession::PrepareOpen(
        const std::filesystem::path& path) const
    {
        if (path.empty() || path.filename().empty())
        {
            throw std::invalid_argument(
                "Open destination must name a scene file.");
        }

        const std::filesystem::path resolvedPath =
            std::filesystem::absolute(
                path.is_absolute()
                ? path
                : m_ProjectRoot / "scenes" / path)
            .lexically_normal();

        const SceneDocument document =
            LoadSceneDocument(
                resolvedPath,
                m_ProjectRoot);

        return EditorSession{
            document,
            m_ProjectRoot,
            resolvedPath
        };
    }

    void EditorSession::Save()
    {
        if (!m_ScenePath)
        {
            throw std::logic_error(
                "Scene has no file path. Save As is required.");
        }

        SaveAs(*m_ScenePath);
    }

    void EditorSession::SaveAs(
        const std::filesystem::path& path)
    {
        if (path.empty() || path.filename().empty())
        {
            throw std::invalid_argument(
                "Save destination must name a file.");
        }

        const SceneDocument document = m_Scene.ToDocument();

        // Prepare state before writing. Commit it only after success.
        std::string changeBaseline =
            SerializeSceneDocument(document);

        std::filesystem::path destination = path;

        if (destination.is_relative())
        {
            const std::filesystem::path sceneDirectory =
                m_ProjectRoot / "scenes";

            std::filesystem::create_directories(sceneDirectory);

            destination = sceneDirectory / destination;
        }

        std::optional<std::filesystem::path> scenePath{
            std::filesystem::absolute(destination).lexically_normal()
        };

        SaveSceneDocument(
            document,
            *scenePath,
            m_ProjectRoot);

        m_ChangeBaseline.swap(changeBaseline);
        m_ScenePath.swap(scenePath);
    }

    std::optional<std::filesystem::path>
        EditorSession::SharedModelPath() const
    {
        std::optional<std::filesystem::path> sharedPath;

        for (const SandboxObject& object : m_Scene.Objects())
        {
            const std::u8string reference{
                object.modelPath.begin(),
                object.modelPath.end()
            };

            const std::filesystem::path resolvedPath =
                std::filesystem::canonical(
                    m_ProjectRoot /
                    std::filesystem::path{ reference });

            if (sharedPath && *sharedPath != resolvedPath)
            {
                throw std::runtime_error(
                    "Opening scenes with multiple different models "
                    "is not supported yet.");
            }

            sharedPath = resolvedPath;
        }

        return sharedPath;
    }
}