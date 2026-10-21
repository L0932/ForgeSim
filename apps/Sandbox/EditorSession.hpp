#pragma once

#include "SandboxScene.hpp"
#include "SceneDocument.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace ForgeSim::Sandbox
{
    class EditorSession final
    {
    public:
        EditorSession(
            const SceneDocument& document,
            const std::filesystem::path& projectRoot,
            std::optional<std::filesystem::path> scenePath =
            std::nullopt);

        [[nodiscard]] SandboxScene& Scene() noexcept;
        [[nodiscard]] const SandboxScene& Scene() const noexcept;

        [[nodiscard]] const std::optional<std::filesystem::path>&
            ScenePath() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& ProjectRoot() const noexcept;

        [[nodiscard]] bool HasUnsavedChanges() const;

        // Accept the current scene as the starting state without saving it.
        void ResetChangeBaseline();

        // Prepare replacement sessions without modifying this session.
        [[nodiscard]] EditorSession PrepareNew() const;

        [[nodiscard]] EditorSession PrepareOpen(
            const std::filesystem::path& path) const;

        // Save requires an existing scene path.
        void Save();

        // Updates the current path only after saving succeeds.
        void SaveAs(const std::filesystem::path& path);

        // Empty scenes have no shared model.
        // Throws if objects reference different models.
        [[nodiscard]]
        std::optional<std::filesystem::path> SharedModelPath() const;

    private:
        SandboxScene m_Scene;
        std::filesystem::path m_ProjectRoot;

        std::optional<std::filesystem::path> m_ScenePath;
        std::string m_ChangeBaseline;
    };
}