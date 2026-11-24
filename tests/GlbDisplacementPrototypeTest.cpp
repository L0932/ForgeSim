#include "GlbDisplacement.hpp"
#include "HeightGlbFixture.hpp"
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <nlohmann/json.hpp>
#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using Json=nlohmann::json;
namespace fs=std::filesystem;
using namespace ForgeSim::Assets;
using ForgeSim::DisplacementPrototype::DisplaceGlb;
namespace
{
    void Require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
    template<typename Action>void Reject(Action action,const char* expected)
    {
        bool failed=false;try{action();}catch(const std::exception& error)
        {failed=std::string{error.what()}.find(expected)!=std::string::npos;}
        Require(failed,"Expected error was not reported");
    }
    using Fixture = ForgeSim::Test::Fixture;
    Json ReadJson(const fs::path& path)
    {
        std::ifstream file{path,std::ios::binary};file.seekg(12);std::uint32_t length=0;file.read(reinterpret_cast<char*>(&length),4);file.seekg(20);
        std::string text(length,' ');file.read(text.data(),static_cast<std::streamsize>(length));return Json::parse(text);
    }
    void PreserveAndDiscover()
    {
        Fixture f;const auto input=f.Write("input.glb"),output=f.directory/"output.glb";
        MeshDisplacementSettings settings;settings.subdivisionLevels=1;settings.stitchSmoothSeams=true;
        const auto report=DisplaceGlb(input,output,{},"",0,0,0,settings);
        Require(report.heightSource=="Wall_disp"&&report.heightBitDepth==16&&report.triangles==4,"Discover matching embedded 16-bit height");
        const auto before=ReadJson(input),after=ReadJson(output);
        for(const char* field:{"materials","images","textures","nodes","scenes"})Require(before[field]==after[field],"Preserve source materials, images and scene transforms");
        Require(after["extras"]["keepMe"]=="unchanged","Preserve unrelated root extras");
        const auto asset=LoadGltfScene(output);
        Require(asset.meshes[0].primitives[0].geometry.vertices.size()==6,"ForgeSim loader must accept refined GLB geometry");
        Require(asset.meshes[0].primitives[0].geometry.vertices[0].position[1]>0,"Height must move real mesh positions");
        Reject([&]{static_cast<void>(DisplaceGlb(output,f.directory/"twice.glb",{},"",0,0,0,settings));},"already height-baked");
        Reject([&]{static_cast<void>(DisplaceGlb(input,output,{},"",0,0,0,settings));},"Output exists");
    }
    void AmbiguityAndExplicitChoice()
    {
        Fixture f;auto other=f.json["images"][0];other["name"]="Wall_height";f.json["images"].push_back(other);
        const auto input=f.Write("ambiguous.glb");MeshDisplacementSettings settings;
        Reject([&]{static_cast<void>(DisplaceGlb(input,f.directory/"bad.glb",{},"",0,0,0,settings));},"Ambiguous");
        Require(!fs::exists(f.directory/"bad.glb"),"Ambiguity must not create an output");
        const auto report=DisplaceGlb(input,f.directory/"chosen.glb",{},"Wall_disp",0,0,0,settings);
        Require(report.heightSource=="Wall_disp","Explicit assignment must override naming candidates");
        Reject([&]{static_cast<void>(DisplaceGlb(input,f.directory/"missing.glb",{},"absent",0,0,0,settings));},"No matching");
        Reject([&]{static_cast<void>(DisplaceGlb(input,f.directory/"two.glb",input,"Wall_disp",0,0,0,settings));},"not both");
    }
}
int main()
{
    try{PreserveAndDiscover();AmbiguityAndExplicitChoice();}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"Embedded height discovery, precision, material preservation, GLB reload and ambiguity checks passed\n";
}
