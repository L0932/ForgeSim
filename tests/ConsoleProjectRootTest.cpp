#include "ConsoleProjectRoot.hpp"
#include "EditorSession.hpp"
#include "SceneDocumentIO.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace {
void Require(bool ok, const char* message) { if(!ok) throw std::runtime_error(message); }
struct TemporaryDirectory {
    fs::path path;
    TemporaryDirectory() {
        path=fs::temp_directory_path()/ ("ForgeSimConsoleRoot-"+
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if(!fs::create_directory(path)) throw std::runtime_error("Temporary directory creation failed");
    }
    ~TemporaryDirectory(){std::error_code error;fs::remove_all(path,error);}
};
}
int main(){
 try {
    TemporaryDirectory temp;
    const auto oldRoot=temp.path/"old root";
    const auto newRoot=temp.path/"new root";
    fs::create_directories(oldRoot);fs::create_directories(newRoot/"Cathedral_01");
    std::ofstream(oldRoot/"old.glb") << "test asset";
    std::ofstream(newRoot/"Cathedral_01/cathedral.glb") << "test asset";
    SceneDocument oldScene;oldScene.name="Original";
    oldScene.objects.push_back({.id=1,.name="Original object",.transform={},.modelPath="old.glb"});
    SceneDocument newScene;newScene.name="Cathedral";
    newScene.objects.push_back({.id=1,.name="Cathedral object",.transform={},.modelPath="Cathedral_01/cathedral.glb"});
    const auto scenePath=temp.path/"cathedral.scene.json";
    SaveSceneDocument(newScene,scenePath,newRoot);
    EditorSession current(oldScene,oldRoot,std::nullopt,temp.path);
    ConsoleProjectRoot setting{current.ProjectRoot()};
    ConsoleCommands commands;
    commands.Register("project.root","Project root","project.root [directory]",0,1,
        [&setting](auto args,const auto&){return setting.Execute(args);});
    const auto utf8=newRoot.generic_u8string();
    const auto command="project.root \""+std::string(utf8.begin(),utf8.end())+"\"";
    Require(!commands.Execute(command).error,"Setting quoted project root");
    Require(setting.Path()==fs::canonical(newRoot),"Canonical root setting");
    Require(commands.Execute("project.root").message.find("new root")!=std::string::npos,"Root query");
    Require(commands.Execute("project.root \"\"").error,"Empty root rejected");
    Require(commands.Execute("project.root \""+(newRoot/"missing").string()+"\"").error,"Missing root rejected");
    Require(commands.Execute("project.root \""+(oldRoot/"old.glb").string()+"\"").error,"File root rejected");
    Require(setting.Path()==fs::canonical(newRoot),"Invalid setting preserved last valid root");
    bool failed=false;
    try { static_cast<void>(current.PrepareOpen(scenePath)); } catch(const std::exception&) { failed=true; }
    Require(failed,"Scene cannot open under the old root");
    const auto replacement=current.PrepareOpen(scenePath,setting.Path());
    Require(replacement.ProjectRoot()==fs::canonical(newRoot),"Replacement adopts new root");
    Require(replacement.SharedModelPath()==fs::canonical(newRoot/"Cathedral_01/cathedral.glb"),"Replacement model resolution");
    Require(current.ProjectRoot()==fs::canonical(oldRoot) && current.Scene().Name()=="Original", "Current scene unchanged before replacement");
    Require(!current.HasUnsavedChanges(),"Root command does not dirty current scene");
    current.SaveAs(temp.path/"original.scene.json");
    Require(LoadSceneDocument(temp.path/"original.scene.json",oldRoot).name=="Original", "Current scene still saves with original root");
    failed=false;
    try {static_cast<void>(current.PrepareOpen(temp.path/"missing.scene.json",setting.Path()));}
    catch(const std::exception&) {failed=true;}
    Require(failed && current.Scene().Name()=="Original" && !current.HasUnsavedChanges(),"Failed open preserves active scene");
    std::cout<<"Runtime project root and scene-open isolation passed\n";
    return 0;
 }catch(const std::exception& exception){std::cerr<<exception.what()<<'\n';return 1;}
}
