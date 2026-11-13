#include "EditorSession.hpp"
#include "ModelImportLocation.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneSerializer.hpp"

#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <utility>

namespace ForgeSim::Sandbox
{
    EditorSession EditorSession::PrepareImportModel(const std::filesystem::path& path) const
    {
        auto extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension != ".glb" && extension != ".gltf")
            throw std::invalid_argument("Import Model supports .glb and .gltf files.");
        const bool adoptRoot = !m_Scene.HasAllocatedObjectIds() && !m_ScenePath &&
            m_Scene.RenderingSettings().environmentSource != SceneEnvironmentSource::ProjectAsset;
        ModelImportLocation location;
        try { location = ResolveModelImportLocation(path,
            adoptRoot ? std::nullopt : std::optional{m_ProjectRoot}); }
        catch (const std::invalid_argument&)
        {
            throw std::invalid_argument("Place the model and its dependencies inside this scene's asset folder: " + m_ProjectRoot.string());
        }
        const auto reference = location.model.lexically_relative(location.root).generic_u8string();
        const auto name = path.stem().generic_u8string();
        EditorSession replacement = *this;
        if (adoptRoot)
        {
            replacement.m_ProjectRoot = location.root;
            replacement.m_DefaultSceneDirectory = location.root / "scenes";
        }
        static_cast<void>(replacement.m_Scene.AddModelObject(
            std::string(name.begin(), name.end()), std::string(reference.begin(), reference.end())));
        return replacement;
    }

    EditorSession::EditorSession(
        const SceneDocument& document,
        const std::filesystem::path& projectRoot,
        std::optional<std::filesystem::path> scenePath,
        std::optional<std::filesystem::path> defaultSceneDirectory)
        : m_Scene(document)
        , m_ProjectRoot(
            std::filesystem::weakly_canonical(
                std::filesystem::absolute(projectRoot)))
        , m_DefaultSceneDirectory(
            std::filesystem::absolute(
                defaultSceneDirectory.value_or(
                    m_ProjectRoot / "scenes"))
            .lexically_normal())
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
        EditorSession::DefaultSceneDirectory() const noexcept
    {
        return m_DefaultSceneDirectory;
    }

    const std::filesystem::path&
        EditorSession::ProjectRoot() const noexcept
    {
        return m_ProjectRoot;
    }

    std::string EditorSession::DisplayName() const
    {
        if (!m_ScenePath) return m_Scene.Name();
        const auto filename = m_ScenePath->filename().generic_u8string();
        return std::string(filename.begin(), filename.end());
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
            m_ProjectRoot,
            std::nullopt,
            m_DefaultSceneDirectory
        };
    }

    EditorSession EditorSession::PrepareOpen(
        const std::filesystem::path& path,
        std::optional<std::filesystem::path> projectRoot) const
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
                : m_DefaultSceneDirectory / path)
            .lexically_normal();

        const auto root = projectRoot
            ? std::filesystem::canonical(std::filesystem::absolute(*projectRoot))
            : m_ProjectRoot;
        if (projectRoot && !std::filesystem::is_directory(root))
            throw std::invalid_argument("Project root must name an existing directory.");

        const SceneDocument document =
            LoadSceneDocument(
                resolvedPath,
                root);

        return EditorSession{
            document,
            root,
            resolvedPath,
            m_DefaultSceneDirectory
        };
    }

    EditorSession EditorSession::PrepareSample(
        const std::filesystem::path& path,
        const std::filesystem::path& bundledRoot) const
    {
        auto sample = PrepareOpen(path, bundledRoot);
        sample.m_ScenePath.reset();
        return sample;
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
            std::filesystem::create_directories(
                m_DefaultSceneDirectory);

            destination = m_DefaultSceneDirectory / destination;
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

    std::vector<std::filesystem::path> EditorSession::ModelPaths() const
    {
        std::vector<std::filesystem::path> paths;

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

            if (std::find(paths.begin(), paths.end(), resolvedPath) == paths.end())
                paths.push_back(resolvedPath);
        }

        return paths;
    }
}