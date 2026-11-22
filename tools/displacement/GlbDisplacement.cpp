#include "GlbDisplacement.hpp"
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cctype>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace ForgeSim::DisplacementPrototype
{
    namespace
    {
        using Json = nlohmann::json;
        namespace fs = std::filesystem;
        using namespace Assets;
        constexpr std::size_t MaxInputBytes = 256 * 1024 * 1024;
        std::uint32_t Word(const std::vector<std::uint8_t>& data, std::size_t offset)
        {
            if (offset > data.size() || data.size() - offset < 4) throw std::invalid_argument("Truncated GLB.");
            return data[offset] | (std::uint32_t(data[offset+1]) << 8) |
                (std::uint32_t(data[offset+2]) << 16) | (std::uint32_t(data[offset+3]) << 24);
        }
        std::vector<std::uint8_t> Read(const fs::path& path)
        {
            std::ifstream file{path, std::ios::binary | std::ios::ate};
            if (!file) throw std::runtime_error("Cannot open input GLB.");
            const auto size = file.tellg();
            if (size < 28 || size > static_cast<std::streamoff>(MaxInputBytes))
                throw std::invalid_argument("Prototype GLB input must be between 28 bytes and 256 MiB.");
            std::vector<std::uint8_t> data(static_cast<std::size_t>(size)); file.seekg(0);
            if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size())))
                throw std::runtime_error("Cannot read input GLB.");
            return data;
        }
        std::string Lower(std::string value)
        {
            for (auto& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            return value;
        }
        bool NamedHeight(const std::string& name, const std::string& material)
        {
            if (material.empty()) return false;
            const auto stem = Lower(fs::path{std::u8string{name.begin(), name.end()}}.stem().string());
            for (const auto* suffix : {"_disp", "_displacement", "_height"})
            {
                const auto prefix = Lower(material) + suffix;
                if (stem == prefix || (stem.starts_with(prefix) && stem.size() > prefix.size() && stem[prefix.size()] == '_'))
                    return true;
            }
            return false;
        }
        struct Container
        {
            Json json;
            std::vector<std::uint8_t> binary;
            explicit Container(const fs::path& input)
            {
                const auto data = Read(input);
                if (Word(data,0) != 0x46546c67 || Word(data,4) != 2 || Word(data,8) != data.size() || Word(data,16) != 0x4e4f534a)
                    throw std::invalid_argument("Expected a valid glTF 2.0 binary container.");
                const std::size_t jsonSize = Word(data,12);
                if (jsonSize > data.size() - 28) throw std::invalid_argument("Truncated GLB JSON.");
                json = Json::parse(data.begin()+20, data.begin()+20+jsonSize);
                const auto binOffset = 20+jsonSize;
                if (Word(data,binOffset+4) != 0x004e4942 || Word(data,binOffset) != data.size() - binOffset - 8)
                    throw std::invalid_argument("Prototype requires one JSON chunk and one embedded BIN chunk.");
                if (!json.contains("buffers") || json["buffers"].size()!=1 || json["buffers"][0].contains("uri"))
                    throw std::invalid_argument("Prototype requires a single embedded GLB buffer.");
                binary.assign(data.begin()+static_cast<std::ptrdiff_t>(binOffset+8), data.end());
                if (json.value("extras",Json::object()).contains("forgeSimHeightBake"))
                    throw std::invalid_argument("This GLB is already height-baked. Use the original to avoid double displacement.");
            }
            std::span<const std::uint8_t> Image(std::size_t imageIndex) const
            {
                const auto& image = json.at("images").at(imageIndex);
                if (!image.contains("bufferView") || image.value("mimeType","") != "image/png")
                    throw std::invalid_argument("Selected height image must be an embedded PNG; otherwise supply --height.");
                const auto& view = json.at("bufferViews").at(image.at("bufferView").get<std::size_t>());
                const auto offset = view.value("byteOffset",std::size_t{}), length = view.at("byteLength").get<std::size_t>();
                if (view.value("buffer",0)!=0 || offset > binary.size() || length > binary.size()-offset)
                    throw std::invalid_argument("Embedded height image range is invalid.");
                return std::span<const std::uint8_t>{binary}.subspan(offset,length);
            }
            std::size_t Attribute(const void* data, std::size_t bytes, std::size_t count, const char* type, unsigned component)
            {
                while (binary.size()%4) binary.push_back(0);
                const auto offset=binary.size(); const auto* start=static_cast<const std::uint8_t*>(data);
                binary.insert(binary.end(),start,start+bytes);
                const auto view=json["bufferViews"].size();
                json["bufferViews"].push_back({{"buffer",0},{"byteOffset",offset},{"byteLength",bytes}});
                const auto accessor=json["accessors"].size();
                json["accessors"].push_back({{"bufferView",view},{"componentType",component},{"count",count},{"type",type}});
                return accessor;
            }
            void Geometry(unsigned meshIndex, unsigned primitiveIndex, const MeshData& mesh)
            {
                std::vector<std::array<float,3>> positions,normals;
                std::vector<std::array<float,2>> uv;
                std::vector<std::array<float,4>> tangents;
                auto minimum=mesh.vertices.front().position,maximum=minimum;
                for(const auto& vertex:mesh.vertices)
                {
                    positions.push_back(vertex.position);normals.push_back(vertex.normal);
                    uv.push_back(vertex.textureCoordinates);tangents.push_back(vertex.tangent);
                    for(unsigned axis=0;axis<3;++axis)
                    {minimum[axis]=std::min(minimum[axis],vertex.position[axis]);maximum[axis]=std::max(maximum[axis],vertex.position[axis]);}
                }
                const auto position=Attribute(positions.data(),positions.size()*sizeof(positions[0]),positions.size(),"VEC3",5126);
                json["accessors"][position]["min"]=minimum;json["accessors"][position]["max"]=maximum;
                const auto normal=Attribute(normals.data(),normals.size()*sizeof(normals[0]),normals.size(),"VEC3",5126);
                const auto texcoord=Attribute(uv.data(),uv.size()*sizeof(uv[0]),uv.size(),"VEC2",5126);
                const auto tangent=Attribute(tangents.data(),tangents.size()*sizeof(tangents[0]),tangents.size(),"VEC4",5126);
                const auto indices=Attribute(mesh.indices.data(),mesh.indices.size()*sizeof(mesh.indices[0]),mesh.indices.size(),"SCALAR",5125);
                auto& primitive=json["meshes"][meshIndex]["primitives"][primitiveIndex];
                primitive["attributes"]={{"POSITION",position},{"NORMAL",normal},{"TEXCOORD_0",texcoord},{"TANGENT",tangent}};
                primitive["indices"]=indices;
            }
            void Save(const fs::path& output)
            {
                static_assert(std::endian::native==std::endian::little);
                if(fs::exists(output))throw std::runtime_error("Output exists; choose a new filename.");
                json["buffers"][0]["byteLength"]=binary.size();
                auto text=json.dump();while(text.size()%4)text+=' ';while(binary.size()%4)binary.push_back(0);
                const auto size=28ull+text.size()+binary.size();
                if(size>std::numeric_limits<std::uint32_t>::max())throw std::length_error("Output exceeds GLB container limit.");
                std::ofstream file{output,std::ios::binary};
                const auto word=[&](std::uint32_t value){file.write(reinterpret_cast<const char*>(&value),4);};
                word(0x46546c67);word(2);word(static_cast<std::uint32_t>(size));
                word(static_cast<std::uint32_t>(text.size()));word(0x4e4f534a);file.write(text.data(),static_cast<std::streamsize>(text.size()));
                word(static_cast<std::uint32_t>(binary.size()));word(0x004e4942);
                file.write(reinterpret_cast<const char*>(binary.data()),static_cast<std::streamsize>(binary.size()));
                file.close();if(!file)throw std::runtime_error("Could not save displaced GLB.");
            }
        };
    }
    Report DisplaceGlb(const fs::path& input,const fs::path& output,const fs::path& externalHeight,
        const std::string& embeddedName,unsigned meshIndex,unsigned primitiveIndex,unsigned channel,
        const MeshDisplacementSettings& settings)
    {
        if(!externalHeight.empty()&&!embeddedName.empty())throw std::invalid_argument("Choose --height or --embedded-height, not both.");
        Container container{input};
        auto& primitive=container.json.at("meshes").at(meshIndex).at("primitives").at(primitiveIndex);
        if(primitive.value("mode",4)!=4 || primitive.contains("targets") || primitive.contains("extensions"))
            throw std::invalid_argument("Prototype supports static, uncompressed triangle primitives without morph targets.");
        for(const auto& [attribute,accessor]:primitive.at("attributes").items())
        {
            static_cast<void>(accessor);
            if(attribute!="POSITION"&&attribute!="NORMAL"&&attribute!="TEXCOORD_0"&&attribute!="TANGENT")
                throw std::invalid_argument("Prototype cannot preserve this primitive attribute: "+attribute);
        }
        if (!primitive.at("attributes").contains("TEXCOORD_0"))
            throw std::invalid_argument("Height displacement requires TEXCOORD_0.");
        const auto asset=LoadGltfScene(input);
        const auto& source=asset.meshes.at(meshIndex).primitives.at(primitiveIndex).geometry;
        HeightField height;Report report;report.sourceVertices=source.vertices.size();
        if(!externalHeight.empty())
        {
            height=LoadHeightPng(externalHeight,channel);report.heightSource=externalHeight.filename().string();
        }
        else
        {
            const auto material=primitive.contains("material") ?
                container.json.at("materials").at(primitive["material"].get<std::size_t>()).value("name","") : std::string{};
            std::vector<std::size_t> candidates;
            for(std::size_t index=0;index<container.json.value("images",Json::array()).size();++index)
            {
                const auto name=container.json["images"][index].value("name","");
                if(!embeddedName.empty()?name==embeddedName:NamedHeight(name,material))candidates.push_back(index);
            }
            if(candidates.empty())throw std::invalid_argument("No matching embedded height image; use --embedded-height NAME or --height PNG.");
            if(candidates.size()!=1)throw std::invalid_argument("Ambiguous height images; choose one with --embedded-height NAME.");
            report.heightSource=container.json["images"][candidates[0]].value("name","");
            const auto image=container.Image(candidates[0]);height=DecodeHeightPng(image,channel,report.heightSource);
            report.heightBitDepth=image[24];
        }
        const auto begin=std::chrono::steady_clock::now();
        const auto displaced=DisplaceMesh(source,height,settings);
        report.generationMilliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
        report.vertices=displaced.vertices.size();report.triangles=displaced.indices.size()/3;
        container.Geometry(meshIndex,primitiveIndex,displaced);
        if(!container.json.contains("extras"))container.json["extras"]=Json::object();
        if(!container.json["extras"].is_object())throw std::invalid_argument("Prototype requires object-valued root extras.");
        container.json["extras"]["forgeSimHeightBake"]={{"baked",true},{"heightImage",report.heightSource},{"channel",channel},
            {"stitchSmoothSeams",settings.stitchSmoothSeams},{"scale",settings.scale},{"midlevel",settings.midlevel},{"inverted",settings.inverted},{"subdivisionLevels",settings.subdivisionLevels}};
        container.Save(output);return report;
    }
}
