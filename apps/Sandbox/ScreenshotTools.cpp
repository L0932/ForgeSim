#include "ScreenshotTools.hpp"
#include "ScreenshotCapture.hpp"
#include "ScreenshotPath.hpp"
#include "SceneViewportPanel.hpp"
#include "SandboxPaths.hpp"
#include "EditorShortcutContext.hpp"
#include <fstream>
#include <utility>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <ForgeSim/Core/Log.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        std::string Utf8(const std::filesystem::path& path)
        { const auto value=path.generic_u8string();return {value.begin(),value.end()}; }
    }
    void ScreenshotTools::Initialize()
    {
        m_SettingsPath=EditorSettingsDirectory()/"screenshots.json";
        m_Directory=m_SettingsPath.parent_path().parent_path()/"Screenshots";
        try
        {
            std::ifstream input(m_SettingsPath);
            const auto settings=nlohmann::json::parse(input,nullptr,false);
            if(settings.is_object() && settings.value("version",0)==SchemaVersion && settings.contains("directory") && settings["directory"].is_string())
            {
                const auto raw=settings["directory"].get<std::string>();
                const std::filesystem::path directory{std::u8string(raw.begin(),raw.end())};
                if(directory.is_absolute()) m_Directory=directory;
            }
        }
        catch(...) { /* Keep the default when a local preference is unavailable. */ }
    }
    void ScreenshotTools::DrawMenu(SceneViewportPanel& viewport,bool actionsAllowed)
    {
        if(!ImGui::BeginMainMenuBar()) return;
        if(ImGui::BeginMenu("Capture"))
        {
            ImGui::BeginDisabled(!actionsAllowed || m_Request.Pending());
            if(ImGui::MenuItem("Scene viewport PNG","F9")) m_Request.Request(ScreenshotTarget::Scene);
            if(ImGui::MenuItem("Entire editor PNG","Ctrl+F9")) m_Request.Request(ScreenshotTarget::Editor);
            ImGui::Separator();
            if(ImGui::MenuItem(viewport.Maximized()?"Restore Scene viewport":"Maximize Scene viewport","Shift+Space"))
                viewport.SetMaximized(!viewport.Maximized());
            if(ImGui::MenuItem("Screenshot folder...")) m_ChooseDirectory=true;
            ImGui::EndDisabled();
            ImGui::Separator();
            ImGui::TextDisabled("PNG output folder:");ImGui::PushTextWrapPos(ImGui::GetCursorPosX()+360);
            ImGui::TextWrapped("%s",Utf8(m_Directory).c_str());ImGui::PopTextWrapPos();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    void ScreenshotTools::HandleShortcuts(bool blocked)
    {
        const auto& io=ImGui::GetIO();
        if(AllowSceneShortcut({.windowFocused=ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow),
            .editingBlocked=blocked,.textInput=io.WantTextInput,.itemActive=ImGui::IsAnyItemActive(),
            .popupOpen=ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel),
            .modifierHeld=io.KeyShift || io.KeyAlt || io.KeySuper}) && ImGui::IsKeyPressed(ImGuiKey_F9,false))
            m_Request.Request(io.KeyCtrl?ScreenshotTarget::Editor:ScreenshotTarget::Scene);
    }
    void ScreenshotTools::DrawFeedback()
    {
        if(m_Request.Pending() || m_Feedback.empty() || std::chrono::steady_clock::now()>=m_FeedbackUntil) return;
        const auto* viewport=ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+viewport->WorkSize.x-16,viewport->WorkPos.y+viewport->WorkSize.y-16),
            ImGuiCond_Always,ImVec2(1,1));
        ImGui::SetNextWindowBgAlpha(.92f);
        ImGui::Begin("Screenshot feedback",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|
            ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoDocking);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX()+400);ImGui::TextWrapped("%s",m_Feedback.c_str());ImGui::PopTextWrapPos();
        ImGui::End();
    }
    void ScreenshotTools::EndFrame(unsigned int sceneTexture,std::uint32_t width,std::uint32_t height,bool sceneDrawable)
    {
        const auto target=m_Request.EndFrame();if(!target) return;
        try
        {
            if(*target==ScreenshotTarget::Scene && !sceneDrawable) throw std::runtime_error("Scene viewport is not drawable.");
            const auto image=*target==ScreenshotTarget::Scene?CaptureSceneTexture(sceneTexture):CaptureEditorBackbuffer(width,height);
            std::filesystem::create_directories(m_Directory);
            const auto path=NextScreenshotPath(m_Directory,*target);WriteScreenshotPng(path,image);
            m_Feedback="Screenshot saved ("+std::to_string(image.width)+" x "+std::to_string(image.height)+"): "+Utf8(path);
            Core::LogInfo(m_Feedback);
        }
        catch(const std::exception& error)
        {m_Feedback="Screenshot failed: "+std::string(error.what());Core::LogError(m_Feedback);}
        m_FeedbackUntil=std::chrono::steady_clock::now()+std::chrono::seconds(6);
    }
    bool ScreenshotTools::TakeDirectoryRequest() noexcept { return std::exchange(m_ChooseDirectory,false); }
    void ScreenshotTools::SetDirectory(const std::filesystem::path& directory)
    {
        m_Directory=std::filesystem::absolute(directory).lexically_normal();
        std::ofstream output(m_SettingsPath);
        output<<nlohmann::json{{"version", SchemaVersion},{"directory",Utf8(m_Directory)}}.dump(2)<<'\n';output.close();
        if(!output) Core::LogWarning("Screenshot folder changed for this session, but could not save the preference.");
    }
}
