#pragma once

#include "SandboxScene.hpp"
#include "SceneDocument.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ForgeSim::Sandbox
{
    class EditorSession final
    {
    public:
        EditorSession(
            const SceneDocument& document,
            const std::filesystem::path& projectRoot,
            std::optional<std::filesystem::path> scenePath =
            std::nullopt,
            std::optional<std::filesystem::path> defaultSceneDirectory =
            std::nullopt);

        [[nodiscard]] SandboxScene& Scene() noexcept;
        [[nodiscard]] const SandboxScene& Scene() const noexcept;

        [[nodiscard]] const std::optional<std::filesystem::path>&
            ScenePath() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& DefaultSceneDirectory() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& ProjectRoot() const noexcept;

        [[nodiscard]] std::string DisplayName() const;
        [[nodiscard]] bool HasUnsavedChanges() const;

        // Accept the current scene as the starting state without saving it.
        void ResetChangeBaseline();

        // Prepare replacement sessions without modifying this session.
        [[nodiscard]] EditorSession PrepareNew() const;

        // An override applies only to the prepared replacement, never this scene.
        [[nodiscard]] EditorSession PrepareOpen(
            const std::filesystem::path& path,
            std::optional<std::filesystem::path> projectRoot = std::nullopt) const;

        // Samples open as copies: Save must ask for a user destination.
        [[nodiscard]] EditorSession PrepareSample(
            const std::filesystem::path& path,
            const std::filesystem::path& bundledRoot) const;

        [[nodiscard]] EditorSession PrepareImportModel(const std::filesystem::path& path) const;

        // Save requires an existing scene path.
        void Save();

        // Updates the current path only after saving succeeds.
        void SaveAs(const std::filesystem::path& path);

        // Unique model files in object order; empty scenes have none.
        [[nodiscard]] std::vector<std::filesystem::path> ModelPaths() const;

    private:
        SandboxScene m_Scene;
        std::filesystem::path m_ProjectRoot;
        std::filesystem::path m_DefaultSceneDirectory;

        std::optional<std::filesystem::path> m_ScenePath;
        std::string m_ChangeBaseline;
    };
}