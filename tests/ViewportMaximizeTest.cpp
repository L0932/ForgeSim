#include "SceneViewportPanel.hpp"
#include <imgui_internal.h>
#include <iostream>
#include <stdexcept>
using namespace ForgeSim::Sandbox;
void Require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
int main()
{
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.ConfigFlags|=ImGuiConfigFlags_DockingEnable;
    io.DisplaySize={1280,720};io.DeltaTime=1.f/60;
    unsigned char* pixels;int width,height;io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height);
    SceneViewportPanel panel;ImGuiID dock=0;SceneViewportState state;bool blocked=false;
    const auto frame=[&]
    {
        ImGui::NewFrame();
        if(!dock)
        {
            dock=ImGui::GetID("ScreenshotTestDock");ImGui::DockBuilderAddNode(dock,ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock,{1280,720});ImGui::DockBuilderSetNodePos(dock,{0,0});
            ImGuiID left=0,right=0;ImGui::DockBuilderSplitNode(dock,ImGuiDir_Left,.6f,&left,&right);
            ImGui::DockBuilderDockWindow("Scene",left);ImGui::DockBuilderDockWindow("Inspector",right);ImGui::DockBuilderFinish(dock);
        }
        ImGui::DockSpaceOverViewport(dock,ImGui::GetMainViewport());
        if(blocked)io.WantTextInput=true;
        state=panel.Begin({1,1});panel.End();
        ImGui::Begin("Inspector");ImGui::TextUnformatted("Placeholder inspector");ImGui::End();ImGui::Render();
    };
    try
    {
        frame();frame();frame();
        const auto normal=state.framebufferExtent;const auto originalDock=ImGui::FindWindowByName("Scene")->DockId;
        Require(originalDock!=0,"Scene should start docked");
        panel.SetMaximized(true);frame();frame();
        Require(state.framebufferExtent.width>normal.width && panel.Maximized(),"Maximized viewport must use the workspace width");
        Require(ImGui::FindWindowByName("Scene")->DockId==originalDock,"Maximize must preserve the docked window's DockId");
        panel.SetMaximized(false);frame();frame();
        Require(state.framebufferExtent.width==normal.width && ImGui::FindWindowByName("Scene")->DockId==originalDock,
            "Restore must preserve the prior layout and render extent");
        ImGui::SetWindowFocus("Scene");frame();
        io.AddKeyEvent(ImGuiMod_Shift,true);io.AddKeyEvent(ImGuiKey_Space,true);frame();
        Require(panel.Maximized(),"Shift+Space should maximize a focused Scene");
        io.AddKeyEvent(ImGuiKey_Space,false);io.AddKeyEvent(ImGuiMod_Shift,false);frame();
        io.AddKeyEvent(ImGuiKey_Escape,true);frame();
        Require(!panel.Maximized() && !state.exitOrbitRequested,"Escape should restore maximized Scene without exiting orbit");
        io.AddKeyEvent(ImGuiKey_Escape,false);frame();
        blocked=true;io.AddKeyEvent(ImGuiMod_Shift,true);io.AddKeyEvent(ImGuiKey_Space,true);frame();
        Require(!panel.Maximized(),"Text input should block viewport maximize shortcut");
        std::cout<<"Viewport maximize, dock restoration, keyboard controls and Escape priority passed\n";
    }
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    ImGui::DestroyContext();
}
