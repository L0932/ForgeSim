#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <vector>
#include <cstdint>
namespace ForgeSim::Test
{
using Json = nlohmann::json;
namespace fs = std::filesystem;
    struct Fixture
    {
        fs::path directory=fs::temp_directory_path()/("ForgeSim-glb-height-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Json json;std::vector<std::uint8_t> binary;
        Fixture()
        {
            fs::create_directories(directory);
            json={{"asset",{{"version","2.0"}}},{"buffers",Json::array()},
                {"bufferViews",Json::array()},{"accessors",Json::array()},
                {"materials",Json::array({{{"name","Wall"},{"doubleSided",true},
                    {"pbrMetallicRoughness",{{"metallicFactor",0},{"roughnessFactor",.8},{"baseColorTexture",{{"index",0}}}}}}})},
                {"textures",Json::array({{{"source",0}}})},
                {"nodes",Json::array({{{"mesh",0},{"translation",{2,3,4}}}})},
                {"scenes",Json::array({{{"nodes",{0}}}})},{"scene",0},{"extras",{{"keepMe","unchanged"}}}};
            const float positions[]{0,0,0,1,0,0,0,0,1},normals[]{0,1,0,0,1,0,0,1,0},uv[]{.25f,.25f,.75f,.25f,.25f,.75f};
            const std::uint32_t indices[]{0,2,1};
            const auto position=Accessor(positions,sizeof(positions),3,"VEC3",5126);
            json["accessors"][position]["min"]={0,0,0};json["accessors"][position]["max"]={1,0,1};
            const auto normal=Accessor(normals,sizeof(normals),3,"VEC3",5126);
            const auto texcoord=Accessor(uv,sizeof(uv),3,"VEC2",5126);
            const auto index=Accessor(indices,sizeof(indices),3,"SCALAR",5125);
            json["meshes"]=Json::array({{{"primitives",Json::array({{{"attributes",{{"POSITION",position},{"NORMAL",normal},{"TEXCOORD_0",texcoord}}},
                {"indices",index},{"material",0}}})}}});
            const std::vector<std::uint8_t> png{137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,2,0,0,0,1,16,0,0,0,0,129,217,252,21,0,0,0,13,73,68,65,84,120,156,99,104,96,104,96,4,0,3,6,1,2,110,180,45,91,0,0,0,0,73,69,78,68,174,66,96,130};
            const auto view=View(png.data(),png.size());
            json["images"]=Json::array({{{"name","Wall_disp"},{"mimeType","image/png"},{"bufferView",view}}});
        }
        ~Fixture(){std::error_code ignored;fs::remove_all(directory,ignored);}
        std::size_t View(const void* data,std::size_t size)
        {
            while(binary.size()%4)binary.push_back(0);
            const auto offset=binary.size();const auto* bytes=static_cast<const std::uint8_t*>(data);
            binary.insert(binary.end(),bytes,bytes+size);const auto view=json["bufferViews"].size();
            json["bufferViews"].push_back({{"buffer",0},{"byteOffset",offset},{"byteLength",size}});return view;
        }
        std::size_t Accessor(const void* data,std::size_t size,std::size_t count,const char* type,unsigned component)
        {
            const auto view=View(data,size),accessor=json["accessors"].size();
            json["accessors"].push_back({{"bufferView",view},{"count",count},{"componentType",component},{"type",type}});return accessor;
        }
        fs::path Write(const char* name)
        {
            json["buffers"]=Json::array({{{"byteLength",binary.size()}}});
            auto text=json.dump();while(text.size()%4)text+=' ';while(binary.size()%4)binary.push_back(0);
            const auto path=directory/name;std::ofstream file{path,std::ios::binary};
            const auto word=[&](std::uint32_t value){file.write(reinterpret_cast<const char*>(&value),4);};
            word(0x46546c67);word(2);word(static_cast<std::uint32_t>(28+text.size()+binary.size()));
            word(static_cast<std::uint32_t>(text.size()));word(0x4e4f534a);file.write(text.data(),static_cast<std::streamsize>(text.size()));
            word(static_cast<std::uint32_t>(binary.size()));word(0x004e4942);file.write(reinterpret_cast<const char*>(binary.data()),static_cast<std::streamsize>(binary.size()));
            return path;
        }
    };
}
