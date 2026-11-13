#pragma once

#include "ModelNodeQuery.hpp"
#include "GizmoTransform.hpp"
#include <glm/gtc/matrix_inverse.hpp>

namespace ForgeSim::Sandbox
{
    // The adjustment lives after the authored local matrix. Inverting only this
    // fixed prefix preserves authored shear and parent edits without decomposing them.
    [[nodiscard]] inline std::optional<glm::mat4> NodeGizmoBase(
        const ModelNodeQuery& query, std::size_t index,
        const glm::mat4& instance, const NodeTransformOverrides& overrides)
    {
        const auto* node = query.Find(index);
        if (!node) return std::nullopt;
        const auto matrices = query.Evaluate(overrides);
        const auto base = instance * (node->parent ? matrices[*node->parent] : glm::mat4{1}) * node->localTransform;
        for (int c=0;c<4;++c) for(int r=0;r<4;++r)
            if (!std::isfinite(base[c][r])) return std::nullopt;
        const double determinant = glm::determinant(glm::dmat4{base});
        if (!std::isfinite(determinant) || determinant==0) return std::nullopt;
        return base;
    }

    [[nodiscard]] inline bool TryApplyNodeGizmoTransform(Transform& adjustment,
        const glm::mat4& base, const glm::mat4& world, GizmoOperation operation)
    {
        const glm::dmat4 prefix{base};
        const double determinant = glm::determinant(prefix);
        if (!std::isfinite(determinant) || determinant==0) return false;
        const glm::mat4 local{glm::inverse(prefix) * glm::dmat4{world}};
        auto candidate = adjustment;
        if (!TryApplyGizmoTransform(candidate,local,operation) || !ValidNodeOverride(candidate)) return false;
        adjustment = candidate;
        return true;
    }
}
