#pragma once
#include "ScreenshotRequest.hpp"
#include <chrono>
#include <filesystem>
#include <string>
#include <cstdint>

namespace ForgeSim::Sandbox
{
    class SceneViewportPanel;
    class ScreenshotTools final
    {
    public:
        static constexpr int SchemaVersion = 1;

        void Initialize();
        void DrawMenu(SceneViewportPanel& viewport,bool actionsAllowed);
        void HandleShortcuts(bool blocked);
        void DrawFeedback();
        // Call after ImGui rendering, before SwapBuffers. Feedback is emitted
        // only after readback, so it cannot appear in this editor screenshot.
        void EndFrame(unsigned int sceneTexture,std::uint32_t width,std::uint32_t height,bool sceneDrawable);
        [[nodiscard]] bool TakeDirectoryRequest() noexcept;
        [[nodiscard]] const std::filesystem::path& Directory() const noexcept { return m_Directory; }
        void SetDirectory(const std::filesystem::path& directory);
    private:
        ScreenshotRequest m_Request;
        std::filesystem::path m_Directory,m_SettingsPath;
        bool m_ChooseDirectory=false;
        std::string m_Feedback;
        std::chrono::steady_clock::time_point m_FeedbackUntil{};
    };
}
