#include <ForgeSim/Assets/MeshDisplacement.hpp>
#include <cmath>
#include <algorithm>
#include <map>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
using namespace ForgeSim::Assets;
namespace
{
    void Require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
    bool Near(float a, float b) { return std::abs(a-b) < 1e-6f; }
    template<typename Action> void Reject(Action action, const char* message)
    {
        bool failed = false; try { action(); } catch (const std::exception&) { failed = true; }
        Require(failed, message);
    }
    MeshData Plane()
    {
        MeshData mesh;
        for (const auto uv : {std::array<float,2>{0,0}, {1,0}, {0,1}, {1,1}})
        {
            MeshVertex vertex; vertex.position = {uv[0],0,uv[1]}; vertex.normal={0,1,0}; vertex.textureCoordinates=uv;
            mesh.vertices.push_back(vertex);
        }
        mesh.indices={0,2,1,1,2,3}; mesh.availableAttributes.normals=true; mesh.availableAttributes.textureCoordinates=true;
        return mesh;
    }
    void Sampling()
    {
        const HeightField image{2,1,{0,1}};
        Require(Near(SampleHeight(image,.25f,.5f),0) && Near(SampleHeight(image,.75f,.5f),1), "Texel centers must preserve linear values");
        Require(Near(SampleHeight(image,.5f,.5f),.5f), "Height sampling must interpolate bilinearly");
        Require(Near(SampleHeight(image,0,.5f),.5f) && Near(SampleHeight(image,1,.5f),.5f), "Repeat must blend both edges");
        Require(Near(SampleHeight(image,0,.5f,TextureWrap::MirroredRepeat),0) &&
            Near(SampleHeight(image,1,.5f,TextureWrap::MirroredRepeat),1), "Mirror must retain boundary texels");
        Require(Near(SampleHeight(image,-10,.5f,TextureWrap::ClampToEdge),0) &&
            Near(SampleHeight(image,10,.5f,TextureWrap::ClampToEdge),1), "Clamp must not wrap");
        const HeightField rows{1,2,{0,1}};
        Require(Near(SampleHeight(rows,.5f,.25f),0) && Near(SampleHeight(rows,.5f,.75f),1), "UV V must address rows from the top");
        Reject([&]{static_cast<void>(SampleHeight(image,std::numeric_limits<float>::infinity(),0));},"Reject nonfinite UVs");
    }
    void Displacement()
    {
        const auto source=Plane(); MeshDisplacementSettings settings;
        const auto raised=DisplaceMesh(source,HeightField{1,1,{1}},settings);
        for (const auto& vertex:raised.vertices)
        {
            Require(Near(vertex.position[1],.03875f),"Scale/midlevel must displace in mesh-local units");
            Require(Near(vertex.normal[1],1),"A flat displaced plane must retain its normal");
            Require(Near(std::abs(vertex.tangent[0]),1),"Displacement must rebuild tangents");
        }
        Require(source.vertices[0].position[1]==0 && source.indices==raised.indices,"Source mesh must remain untouched");
        auto withoutNormals = source;
        withoutNormals.availableAttributes.normals = false;
        Require(Near(DisplaceMesh(withoutNormals,HeightField{1,1,{1}},settings).vertices[0].position[1],.03875f),
            "Missing source normals must be generated before displacement");
        settings.inverted=true;
        Require(Near(DisplaceMesh(source,HeightField{1,1,{1}},settings).vertices[0].position[1],-.01125f),"Inversion must happen before midlevel subtraction");
        settings.inverted=false; settings.subdivisionLevels=2;
        const auto refined=DisplaceMesh(source,HeightField{1,1,{.225f}},settings);
        Require(refined.vertices.size()==25 && refined.indices.size()==96,"Shared edge subdivision must not duplicate midpoints");
        Require(refined.availableAttributes.normals && refined.availableAttributes.tangents,"Rebuilt attributes must be marked available");
        settings.subdivisionLevels=3; settings.wrapU=settings.wrapV=TextureWrap::ClampToEdge;
        const HeightField bump{3,3,{0,0,0,0,1,0,0,0,0}};
        const auto detailed=DisplaceMesh(source,bump,settings);
        float low=1,high=-1; bool tilted=false;
        for (const auto& vertex:detailed.vertices)
        {
            low=std::min(low,vertex.position[1]); high=std::max(high,vertex.position[1]);
            tilted |= std::abs(vertex.normal[0])>.001f || std::abs(vertex.normal[2])>.001f;
            const float length=std::sqrt(vertex.normal[0]*vertex.normal[0]+vertex.normal[1]*vertex.normal[1]+vertex.normal[2]*vertex.normal[2]);
            Require(Near(length,1),"Displaced normals must be finite unit vectors");
        }
        Require(Near(low,-.01125f) && Near(high,.03875f) && tilted,"Height must alter silhouette bounds and surface normals");
        settings.scale=0; settings.subdivisionLevels=8;
        const auto unchanged=DisplaceMesh(source,{},settings);
        Require(unchanged.vertices.size()==source.vertices.size() && unchanged.indices==source.indices &&
            unchanged.vertices[0].position==source.vertices[0].position && unchanged.vertices[0].tangent==source.vertices[0].tangent,
            "Zero scale must preserve existing geometry and authored attributes");
        settings.scale=.05f; settings.enabled=false;
        Require(DisplaceMesh(source,{},settings).indices==source.indices,"Disabled displacement must require no height texture");
    }
    void HeightUvMapping()
    {
        auto source = Plane();
        for (auto& vertex : source.vertices)
            for (float& uv : vertex.textureCoordinates) uv = .125f + .25f * uv;
        const HeightField height{2,2,{0,.25f,.75f,1}};
        MeshDisplacementSettings settings; settings.scale=1; settings.midlevel=0;
        settings.heightUvTransform.scale={2,2};
        const auto scaled=DisplaceMesh(source,height,settings);
        const float expected[]{0,.25f,.75f,1};
        for (unsigned i=0;i<4;++i)
        {
            Require(Near(scaled.vertices[i].position[1],expected[i]),"Height UV scale must reach known texel centers");
            Require(scaled.vertices[i].textureCoordinates==source.vertices[i].textureCoordinates,"Height mapping must preserve rendering UVs");
        }
        settings.heightUvTransform.offset={.5f,0};
        const auto offset=DisplaceMesh(source,height,settings);
        Require(Near(offset.vertices[0].position[1],.25f) && Near(offset.vertices[1].position[1],0),"Height UV offset must apply before wrapping");
        settings.heightUvTransform.rotationRadians=1.57079632679f;
        settings.heightUvTransform.offset={1,0};
        const auto rotated=DisplaceMesh(source,height,settings);
        Require(Near(rotated.vertices[0].position[1],.25f) && Near(rotated.vertices[1].position[1],1) &&
            Near(rotated.vertices[2].position[1],0),"Height mapping must scale, then rotate, then translate");
        settings.heightUvTransform.scale[0]=std::numeric_limits<float>::quiet_NaN();
        Reject([&]{static_cast<void>(DisplaceMesh(source,height,settings));},"Reject nonfinite height mapping");
    }
    void SeamStitching()
    {
        const auto plane=Plane();MeshData split;
        for(auto index:plane.indices)split.vertices.push_back(plane.vertices[index]);
        split.indices={0,1,2,3,4,5};split.availableAttributes=plane.availableAttributes;
        for(unsigned index=0;index<6;++index)split.vertices[index].textureCoordinates={index<3?0.f:1.f,.5f};
        MeshDisplacementSettings settings;settings.wrapU=TextureWrap::ClampToEdge;
        const HeightField height{2,1,{0,1}};
        const auto open=DisplaceMesh(split,height,settings);
        Require(!Near(open.vertices[1].position[1],open.vertices[4].position[1]),"Unstitched height discontinuity must remain explicit");
        settings.stitchSmoothSeams=true;
        const auto closed=DisplaceMesh(split,height,settings);
        Require(closed.vertices[1].position==closed.vertices[4].position &&
            closed.vertices[2].position==closed.vertices[3].position,"Smooth seam positions must close even with discontinuous height samples");
        Require(closed.vertices[1].normal==closed.vertices[4].normal &&
            closed.vertices[1].textureCoordinates!=closed.vertices[4].textureCoordinates,"Stitch normals while preserving UV splits");
        Require(Near(closed.vertices[1].position[1],.01375f),"Smooth seam policy must average displacement");
        settings.subdivisionLevels = 3;
        const auto refined = DisplaceMesh(split,height,settings);
        std::map<std::array<std::array<float,3>,2>, unsigned> geometricEdges;
        for (std::size_t i = 0; i < refined.indices.size(); i += 3)
            for (unsigned edge = 0; edge < 3; ++edge)
            {
                std::array<std::array<float,3>,2> endpoints{
                    refined.vertices[refined.indices[i+edge]].position,
                    refined.vertices[refined.indices[i+(edge+1)%3]].position};
                std::sort(endpoints.begin(),endpoints.end());
                ++geometricEdges[endpoints];
            }
        unsigned boundary = 0;
        for (const auto& [edge, uses] : geometricEdges) { static_cast<void>(edge); boundary += uses == 1; }
        Require(boundary == 4 * (1u << settings.subdivisionLevels),"Refined UV split must not add internal open edges");
        settings.subdivisionLevels = 0;
        for(unsigned index=3;index<6;++index)split.vertices[index].normal={1,0,0};
        const auto hard=DisplaceMesh(split,height,settings);
        Require(hard.vertices[1].position!=hard.vertices[4].position,"Different source normals must retain hard-boundary identity");
    }
    void Validation()
    {
        auto mesh=Plane(); const HeightField height{1,1,{1}}; MeshDisplacementSettings settings;
        settings.subdivisionLevels=8; settings.maxTriangles=128;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Triangle budget must reject before subdivision");
        settings={}; settings.subdivisionLevels=2; settings.maxVertices=5;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Vertex budget must reject before subdivision");
        settings={}; mesh.indices[0]=999;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Invalid indices must be rejected");
        mesh=Plane(); mesh.availableAttributes.textureCoordinates=false;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Missing UVs must be rejected");
        mesh=Plane();
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,HeightField{1,1,{std::numeric_limits<float>::quiet_NaN()}},settings));},"Reject nonfinite height");
        settings.midlevel=2;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Reject invalid midlevel");
        settings={}; settings.subdivisionLevels=9;
        Reject([&]{static_cast<void>(DisplaceMesh(mesh,height,settings));},"Detail must be hard bounded");
    }
    void ImagePrecision()
    {
        const std::vector<std::uint8_t> png16{137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,2,0,0,0,1,16,0,0,0,0,129,217,252,21,0,0,0,13,73,68,65,84,120,156,99,104,96,104,96,4,0,3,6,1,2,110,180,45,91,0,0,0,0,73,69,78,68,174,66,96,130};
        const std::vector<std::uint8_t> png8{137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,1,0,0,0,1,8,6,0,0,0,31,21,196,137,0,0,0,13,73,68,65,84,120,156,99,48,74,153,118,2,0,3,237,1,245,180,80,229,25,0,0,0,0,73,69,78,68,174,66,96,130};
        const auto height = DecodeHeightPng(png16);
        Require(height.samples.size()==2 && height.samples[1]>height.samples[0] &&
            Near(height.samples[1]-height.samples[0],1.f/65535.f), "16-bit PNG must not be quantized to 8-bit");
        Require(Near(DecodeHeightPng(png8,1).samples[0],100.f/255.f), "Channel extraction must preserve linear data");
        Require(Near(DecodeHeightPng(png8,3).samples[0],200.f/255.f), "Alpha channel must be selectable");
        Reject([&]{static_cast<void>(DecodeHeightPng(png16,1));}, "Absent channel must be rejected");
        Reject([&]{static_cast<void>(DecodeHeightPng({}));}, "Invalid height image must be rejected");
    }
}
int main()
{
    try { Sampling(); Displacement(); HeightUvMapping(); SeamStitching(); Validation(); ImagePrecision(); }
    catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
    std::cout<<"Height precision, sampling, displacement, normals, subdivision, budgets and no-op checks passed\n";
}
