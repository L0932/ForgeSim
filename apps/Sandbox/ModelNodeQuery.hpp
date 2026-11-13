#pragma once

#include "SceneNodeTransform.hpp"
#include "SceneSelection.hpp"
#include <ForgeSim/Renderer/Picking.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace ForgeSim::Sandbox
{
    // Immutable per-asset metadata. Instance transforms are applied at query time.
    class ModelNodeQuery final
    {
    public:
        struct Node
        {
            bool reachable = false;
            std::optional<std::size_t> parent;
            glm::mat4 assetTransform{1};
            std::optional<Renderer::AxisAlignedBoundingBox> subtreeBounds;
            std::size_t begin = 0, end = 0;
        };
        explicit ModelNodeQuery(const Assets::SceneAsset& asset) : m_Nodes(asset.nodes.size())
        {
            m_MeshBounds.resize(asset.meshes.size());
            for (std::size_t i = 0; i < asset.meshes.size(); ++i)
                for (const auto& primitive : asset.meshes[i].primitives)
                    for (const auto& vertex : primitive.geometry.vertices)
                        Expand(m_MeshBounds[i], {vertex.position[0], vertex.position[1], vertex.position[2]});
            std::size_t cursor = 0;
            for (const auto root : asset.rootNodeIndices) Visit(asset, root, std::nullopt, glm::mat4{1}, cursor, 0);
        }
        [[nodiscard]] const Node* Find(std::size_t index) const noexcept
        { return index < m_Nodes.size() && m_Nodes[index].reachable ? &m_Nodes[index] : nullptr; }
        [[nodiscard]] bool Contains(std::size_t root, std::size_t descendant) const noexcept
        {
            const auto* parent = Find(root); const auto* child = Find(descendant);
            return parent && child && child->begin >= parent->begin && child->begin < parent->end;
        }
        [[nodiscard]] std::optional<Renderer::AxisAlignedBoundingBox> WorldBounds(
            std::size_t index, const glm::mat4& instance) const
        {
            const auto* node = Find(index);
            return node && node->subtreeBounds ? std::optional{TransformBounds(*node->subtreeBounds, instance)} : std::nullopt;
        }
        [[nodiscard]] std::optional<ModelNodeHit> Pick(const Assets::SceneAsset& asset,
            const Renderer::Ray& ray, const glm::mat4& instance) const
        {
            std::optional<ModelNodeHit> closest;
            for (std::size_t index = 0; index < m_Nodes.size(); ++index)
            {
                const auto* node = Find(index);
                if (!node || !asset.nodes[index].meshIndex) continue;
                const auto meshIndex = *asset.nodes[index].meshIndex;
                if (!m_MeshBounds[meshIndex]) continue;
                const auto matrix = instance * node->assetTransform;
                if (!Renderer::IntersectRayTransformedBox(ray, *m_MeshBounds[meshIndex], matrix)) continue;
                for (const auto& primitive : asset.meshes[meshIndex].primitives)
                {
                    const auto& mesh = primitive.geometry;
                    const auto count = mesh.indices.empty() ? mesh.vertices.size() : mesh.indices.size();
                    for (std::size_t triangle = 0; triangle + 2 < count; triangle += 3)
                    {
                        glm::vec3 vertices[3];
                        bool valid = true;
                        for (std::size_t corner = 0; corner < 3; ++corner)
                        {
                            const auto vertexIndex = mesh.indices.empty() ? triangle + corner : mesh.indices[triangle + corner];
                            if (vertexIndex >= mesh.vertices.size()) { valid = false; break; }
                            const auto& p = mesh.vertices[vertexIndex].position;
                            vertices[corner] = glm::vec3{matrix * glm::vec4{p[0], p[1], p[2], 1}};
                        }
                        if (!valid) continue;
                        const auto hit = IntersectTriangle(ray, vertices[0], vertices[1], vertices[2]);
                        if (hit && (!closest || *hit < closest->distance)) closest = ModelNodeHit{index, *hit};
                    }
                }
            }
            return closest;
        }
    private:
        static void Expand(std::optional<Renderer::AxisAlignedBoundingBox>& bounds, const glm::vec3& point)
        {
            for (int axis = 0; axis < 3; ++axis)
                if (!std::isfinite(point[axis])) throw std::invalid_argument("Node bounds contain non-finite coordinates.");
            if (!bounds) bounds = Renderer::AxisAlignedBoundingBox{point, point};
            else { bounds->minimum = glm::min(bounds->minimum, point); bounds->maximum = glm::max(bounds->maximum, point); }
        }
        static Renderer::AxisAlignedBoundingBox TransformBounds(const Renderer::AxisAlignedBoundingBox& bounds,
            const glm::mat4& transform)
        {
            std::optional<Renderer::AxisAlignedBoundingBox> result;
            for (unsigned int corner = 0; corner < 8; ++corner)
            {
                const glm::vec3 p{corner & 1u ? bounds.maximum.x : bounds.minimum.x,
                    corner & 2u ? bounds.maximum.y : bounds.minimum.y,
                    corner & 4u ? bounds.maximum.z : bounds.minimum.z};
                Expand(result, glm::vec3{transform * glm::vec4{p, 1}});
            }
            return *result;
        }
        void Visit(const Assets::SceneAsset& asset, std::size_t index, std::optional<std::size_t> parent,
            const glm::mat4& transform, std::size_t& cursor, std::size_t depth)
        {
            if (index >= asset.nodes.size() || depth >= 128 || m_Nodes[index].reachable)
                throw std::invalid_argument("Invalid, cyclic, or excessively deep model node hierarchy.");
            auto& target = m_Nodes[index];
            target.reachable = true; target.parent = parent; target.begin = cursor++;
            const auto& source = asset.nodes[index];
            target.assetTransform = transform * CreateSceneNodeLocalMatrix(source.localTransform);
            if (source.meshIndex)
            {
                if (*source.meshIndex >= m_MeshBounds.size()) throw std::invalid_argument("Invalid node mesh reference.");
                if (const auto& meshBounds = m_MeshBounds[*source.meshIndex])
                    target.subtreeBounds = TransformBounds(*meshBounds, target.assetTransform);
            }
            for (const auto child : source.childIndices)
            {
                Visit(asset, child, index, target.assetTransform, cursor, depth + 1);
                if (const auto& childBounds = m_Nodes[child].subtreeBounds)
                { Expand(target.subtreeBounds, childBounds->minimum); Expand(target.subtreeBounds, childBounds->maximum); }
            }
            target.end = cursor;
        }
        static std::optional<float> IntersectTriangle(const Renderer::Ray& ray,
            const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
        {
            // Double-sided geometry picking; alpha textures are intentionally not sampled.
            const glm::dvec3 edge1 = glm::dvec3{b} - glm::dvec3{a};
            const glm::dvec3 edge2 = glm::dvec3{c} - glm::dvec3{a};
            const auto cross = glm::cross(glm::dvec3{ray.direction}, edge2);
            const double determinant = glm::dot(edge1, cross);
            const double tolerance = 1e-12 * glm::length(edge1) * glm::length(edge2) * glm::length(glm::dvec3{ray.direction});
            if (std::abs(determinant) <= tolerance) return std::nullopt;
            const auto origin = glm::dvec3{ray.origin} - glm::dvec3{a};
            const double u = glm::dot(origin, cross) / determinant;
            const auto q = glm::cross(origin, edge1);
            const double v = glm::dot(glm::dvec3{ray.direction}, q) / determinant;
            const double distance = glm::dot(edge2, q) / determinant;
            if (u < 0 || v < 0 || u + v > 1 || distance < 0 || !std::isfinite(distance) ||
                distance > std::numeric_limits<float>::max()) return std::nullopt;
            return static_cast<float>(distance);
        }
        std::vector<Node> m_Nodes;
        std::vector<std::optional<Renderer::AxisAlignedBoundingBox>> m_MeshBounds;
    };
}
