#include <ForgeSim/Assets/MeshDisplacement.hpp>
#include <ForgeSim/Assets/MeshProcessing.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Assets
{
    namespace
    {
        void ValidateHeight(const HeightField& image)
        {
            if (!image.width || !image.height || image.width > 16384 || image.height > 16384 ||
                static_cast<std::uint64_t>(image.width) * image.height > 16'777'216 ||
                image.samples.size() != static_cast<std::size_t>(image.width) * image.height)
                throw std::invalid_argument("Height field dimensions or sample count are invalid (maximum 16M pixels).");
            for (float sample : image.samples)
                if (!std::isfinite(sample) || sample < 0 || sample > 1)
                    throw std::invalid_argument("Height samples must be finite linear values in [0, 1].");
        }
        bool ValidWrap(TextureWrap wrap)
        {
            return wrap == TextureWrap::Repeat || wrap == TextureWrap::MirroredRepeat || wrap == TextureWrap::ClampToEdge;
        }
        glm::vec3 Vector(const std::array<float, 3>& value) { return {value[0], value[1], value[2]}; }
        std::array<float, 3> Array(glm::vec3 value) { return {value.x, value.y, value.z}; }
        glm::vec3 Unit(glm::vec3 value)
        {
            const float length = glm::length(value);
            if (!std::isfinite(length) || length < 1e-12f)
                throw std::invalid_argument("Displacement requires finite, nonzero surface normals.");
            return value / length;
        }
        int Texel(int index, int size, TextureWrap wrap)
        {
            if (wrap == TextureWrap::ClampToEdge) return std::clamp(index, 0, size - 1);
            const int period = wrap == TextureWrap::Repeat ? size : size * 2;
            index = (index % period + period) % period;
            return index < size ? index : period - 1 - index;
        }
        double Coordinate(float value, TextureWrap wrap)
        {
            if (!std::isfinite(value)) throw std::invalid_argument("Height UV coordinates must be finite.");
            if (wrap == TextureWrap::ClampToEdge) return std::clamp(static_cast<double>(value), 0.0, 1.0);
            const double period = wrap == TextureWrap::Repeat ? 1.0 : 2.0;
            double result = std::fmod(static_cast<double>(value), period);
            return result < 0 ? result + period : result;
        }
        float Sample(const HeightField& image, float u, float v, TextureWrap wrapU, TextureWrap wrapV)
        {
            const double x = Coordinate(u, wrapU) * image.width - .5;
            const double y = Coordinate(v, wrapV) * image.height - .5;
            const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
            const float fx = static_cast<float>(x - ix), fy = static_cast<float>(y - iy);
            const auto at = [&](int px, int py)
            {
                return image.samples[static_cast<std::size_t>(Texel(py, static_cast<int>(image.height), wrapV)) * image.width +
                    static_cast<std::size_t>(Texel(px, static_cast<int>(image.width), wrapU))];
            };
            return std::lerp(std::lerp(at(ix, iy), at(ix + 1, iy), fx),
                std::lerp(at(ix, iy + 1), at(ix + 1, iy + 1), fx), fy);
        }
    }

    float SampleHeight(const HeightField& height, float u, float v, TextureWrap wrapU, TextureWrap wrapV)
    {
        ValidateHeight(height);
        if (!ValidWrap(wrapU) || !ValidWrap(wrapV)) throw std::invalid_argument("Invalid height sampler wrap mode.");
        return Sample(height, u, v, wrapU, wrapV);
    }

    MeshData DisplaceMesh(const MeshData& source, const HeightField& height, const MeshDisplacementSettings& settings)
    {
        if (!settings.enabled || settings.scale == 0) return source;
        if (!std::isfinite(settings.scale) || !std::isfinite(settings.midlevel) ||
            settings.midlevel < 0 || settings.midlevel > 1 || settings.subdivisionLevels > 8 ||
            !settings.maxVertices || !settings.maxTriangles || !ValidWrap(settings.wrapU) || !ValidWrap(settings.wrapV))
            throw std::invalid_argument("Invalid displacement settings (maximum eight subdivision levels).");
        ValidateHeight(height);
        if (!source.availableAttributes.textureCoordinates || source.vertices.empty() || source.indices.empty() ||
            source.indices.size() % 3 != 0)
            throw std::invalid_argument("Displacement requires indexed triangles and UV0 coordinates.");
        std::size_t triangles = source.indices.size() / 3;
        std::size_t vertexBound = source.vertices.size();
        if (vertexBound > settings.maxVertices || vertexBound > std::numeric_limits<std::uint32_t>::max() ||
            triangles > settings.maxTriangles)
            throw std::length_error("Source mesh exceeds the displacement geometry budget.");
        for (unsigned level = 0; level < settings.subdivisionLevels; ++level)
        {
            if (triangles > settings.maxTriangles / 4 || triangles > (settings.maxVertices - vertexBound) / 3)
                throw std::length_error("Subdivision exceeds the displacement geometry budget; reduce detail.");
            vertexBound += triangles * 3;
            triangles *= 4;
        }
        if (vertexBound > std::numeric_limits<std::uint32_t>::max() ||
            triangles > std::numeric_limits<std::size_t>::max() / 3)
            throw std::length_error("Displaced mesh exceeds supported index limits.");
        for (auto index : source.indices)
            if (index >= source.vertices.size()) throw std::invalid_argument("Displacement mesh index is out of range.");
        for (const auto& vertex : source.vertices)
        {
            for (float position : vertex.position)
                if (!std::isfinite(position)) throw std::invalid_argument("Displacement positions must be finite.");
            for (float uv : vertex.textureCoordinates)
                if (!std::isfinite(uv)) throw std::invalid_argument("Displacement UV coordinates must be finite.");
        }
        MeshData result = source;
        GenerateMissingNormalsAndTangents(result, false);
        for (auto& vertex : result.vertices) vertex.normal = Array(Unit(Vector(vertex.normal)));
        for (unsigned level = 0; level < settings.subdivisionLevels; ++level)
        {
            std::unordered_map<std::uint64_t, std::uint32_t> edges;
            std::vector<std::uint32_t> indices;
            indices.reserve(result.indices.size() * 4);
            const auto midpoint = [&](std::uint32_t a, std::uint32_t b)
            {
                const auto key = (static_cast<std::uint64_t>(std::min(a, b)) << 32) | std::max(a, b);
                if (const auto found = edges.find(key); found != edges.end()) return found->second;
                const auto first = result.vertices[a], second = result.vertices[b];
                MeshVertex vertex;
                for (unsigned axis = 0; axis < 3; ++axis)
                {
                    vertex.position[axis] = first.position[axis] * .5f + second.position[axis] * .5f;
                    vertex.color[axis] = first.color[axis] * .5f + second.color[axis] * .5f;
                }
                for (unsigned axis = 0; axis < 2; ++axis)
                    vertex.textureCoordinates[axis] = first.textureCoordinates[axis] * .5f + second.textureCoordinates[axis] * .5f;
                vertex.normal = Array(Unit(Vector(first.normal) + Vector(second.normal)));
                const auto index = static_cast<std::uint32_t>(result.vertices.size());
                result.vertices.push_back(vertex);
                edges.emplace(key, index);
                return index;
            };
            for (std::size_t index = 0; index < result.indices.size(); index += 3)
            {
                const auto a = result.indices[index], b = result.indices[index + 1], c = result.indices[index + 2];
                const auto ab = midpoint(a, b), bc = midpoint(b, c), ca = midpoint(c, a);
                indices.insert(indices.end(), {a, ab, ca, ab, b, bc, ca, bc, c, ab, bc, ca});
            }
            result.indices = std::move(indices);
        }
        std::vector<std::uint32_t> seamGroup;
        std::vector<glm::dvec3> seamSum;
        std::vector<unsigned> seamCount;
        if (settings.stitchSmoothSeams)
        {
            struct KeyHash
            {
                std::size_t operator()(const std::array<float,6>& key) const noexcept
                {
                    std::size_t hash=0;
                    for(float value:key) hash ^= std::hash<float>{}(value)+0x9e3779b9u+(hash<<6)+(hash>>2);
                    return hash;
                }
            };
            std::unordered_map<std::array<float,6>,std::uint32_t,KeyHash> groups;
            seamGroup.reserve(result.vertices.size());
            seamSum.resize(result.vertices.size(),glm::dvec3{0});
            seamCount.resize(result.vertices.size(),0);
            for(std::size_t index=0;index<result.vertices.size();++index)
            {
                const auto& vertex=result.vertices[index];
                const std::array<float,6> key{vertex.position[0],vertex.position[1],vertex.position[2],
                    vertex.normal[0],vertex.normal[1],vertex.normal[2]};
                const auto [entry,inserted]=groups.emplace(key,static_cast<std::uint32_t>(index));
                static_cast<void>(inserted);seamGroup.push_back(entry->second);
            }
        }
        for (auto& vertex : result.vertices)
        {
            float sample = Sample(height, vertex.textureCoordinates[0], vertex.textureCoordinates[1], settings.wrapU, settings.wrapV);
            if (settings.inverted) sample = 1 - sample;
            const auto position = Vector(vertex.position) + Vector(vertex.normal) * ((sample - settings.midlevel) * settings.scale);
            for (unsigned axis = 0; axis < 3; ++axis)
                if (!std::isfinite(position[axis])) throw std::invalid_argument("Displacement produced a nonfinite position.");
            vertex.position = Array(position);
        }
        if (settings.stitchSmoothSeams)
        {
            for(std::size_t index=0;index<result.vertices.size();++index)
            {
                seamSum[seamGroup[index]] += glm::dvec3{Vector(result.vertices[index].position)};
                ++seamCount[seamGroup[index]];
            }
            for(std::size_t index=0;index<result.vertices.size();++index)
                result.vertices[index].position=Array(glm::vec3{seamSum[seamGroup[index]]/static_cast<double>(seamCount[seamGroup[index]])});
        }
        result.availableAttributes.normals = false;
        result.availableAttributes.tangents = false;
        GenerateMissingNormalsAndTangents(result, false);
        if (settings.stitchSmoothSeams)
        {
            std::fill(seamSum.begin(),seamSum.end(),glm::dvec3{0});
            for(std::size_t index=0;index<result.indices.size();index+=3)
            {
                const auto a=result.indices[index],b=result.indices[index+1],c=result.indices[index+2];
                const glm::dvec3 pa{Vector(result.vertices[a].position)},pb{Vector(result.vertices[b].position)},pc{Vector(result.vertices[c].position)};
                const auto normal=glm::cross(pb-pa,pc-pa);
                for(auto vertex:{a,b,c})seamSum[seamGroup[vertex]]+=normal;
            }
            for(std::size_t index=0;index<result.vertices.size();++index)
                result.vertices[index].normal=Array(Unit(glm::vec3{seamSum[seamGroup[index]]}));
        }
        GenerateMissingNormalsAndTangents(result, true);
        return result;
    }
}
