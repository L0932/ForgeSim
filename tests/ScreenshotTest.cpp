#include "ScreenshotImage.hpp"
#include "ScreenshotPath.hpp"
#include "ScreenshotRequest.hpp"
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
    template<class Action> void Reject(Action action)
    {bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}Require(rejected,"Invalid screenshot operation was accepted");}
    std::vector<unsigned char> Read(const std::filesystem::path& path)
    {std::ifstream stream(path,std::ios::binary);return {std::istreambuf_iterator<char>(stream),{}};}
    void Queue()
    {
        ScreenshotRequest request;
        Require(!request.Pending() && !request.EndFrame(),"Screenshot queue starts idle");
        Require(request.Request(ScreenshotTarget::Scene),"Scene capture can be requested");
        Require(!request.Request(ScreenshotTarget::Editor),"A second request cannot replace an in-flight screenshot");
        Require(!request.EndFrame() && request.Pending(),"Menu frame must not be captured");
        Require(request.EndFrame()==ScreenshotTarget::Scene && !request.Pending(),"Capture occurs after a clean frame");
        Require(request.Request(ScreenshotTarget::Editor) && !request.EndFrame() && request.EndFrame()==ScreenshotTarget::Editor,
            "Editor capture has the same feedback-safe timing");
    }
    void PngAndPaths(const std::filesystem::path& directory)
    {
        ScreenshotImage image{2,2,{
            0,0,255,0, 255,255,0,0,
            255,0,0,0, 0,255,0,0}};
        NormalizeScreenshot(image);
        Require(image.rgba[0]==255 && image.rgba[1]==0 && image.rgba[3]==255 && image.rgba[11]==255,
            "Readback rows must be top-down and opaque");
        const auto path=NextScreenshotPath(directory,ScreenshotTarget::Scene,std::chrono::system_clock::time_point{});
        WriteScreenshotPng(path,image);
        const auto encoded=Read(path);int width=0,height=0,channels=0;
        auto* decoded=stbi_load_from_memory(encoded.data(),static_cast<int>(encoded.size()),&width,&height,&channels,4);
        Require(decoded && width==2 && height==2,"Screenshot PNG dimensions did not round trip");
        const bool equal=std::equal(image.rgba.begin(),image.rgba.end(),decoded);stbi_image_free(decoded);
        Require(equal,"PNG colors, orientation or alpha changed during encoding");
        const auto collision=NextScreenshotPath(directory,ScreenshotTarget::Scene,std::chrono::system_clock::time_point{});
        Require(collision!=path && collision.extension()==".png","Duplicate timestamps must generate another filename");
        const auto editor=NextScreenshotPath(directory,ScreenshotTarget::Editor,std::chrono::system_clock::time_point{});
        Require(editor.filename().string().find("Editor")!=std::string::npos,"Editor screenshots must identify their capture target");
        Reject([&]{WriteScreenshotPng(path,image);});Require(Read(path)==encoded,"An existing screenshot was overwritten");
        Reject([&]{WriteScreenshotPng(directory/"missing"/"file.png",image);});
        Reject([&]{static_cast<void>(ScreenshotByteCount(0,1));});
        Reject([&]{static_cast<void>(ScreenshotByteCount(std::numeric_limits<std::uint32_t>::max(),2));});
        ScreenshotImage mismatch{2,2,{0}};
        Reject([&]{NormalizeScreenshot(mismatch);});Reject([&]{WriteScreenshotPng(directory/"bad.png",mismatch);});
        Require(!std::filesystem::exists(directory/"bad.png"),"Rejected images must not leave partial output");
        ScreenshotImage odd{1,3,{0,0,255,1, 0,255,0,2, 255,0,0,3}};NormalizeScreenshot(odd);
        Require(odd.rgba[0]==255 && odd.rgba[5]==255 && odd.rgba[10]==255,"Odd-height images must preserve the middle row");
    }
}
int main()
{
    const auto directory=std::filesystem::temp_directory_path()/("ForgeSimScreenshot-"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))/std::filesystem::path{u8"capture-\u00e9-\u68cb"};
    try
    {
        std::filesystem::create_directories(directory);Queue();PngAndPaths(directory);
        std::filesystem::remove_all(directory.parent_path());
    }
    catch(const std::exception& error){std::error_code ignored;std::filesystem::remove_all(directory.parent_path(),ignored);std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"Screenshot PNG, orientation, Unicode paths, collision, validation and request timing checks passed\n";
}
