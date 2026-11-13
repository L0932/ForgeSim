#pragma once

#include "SceneEditHistory.hpp"
#include "GizmoTransform.hpp"
#include <cmath>
#include <glm/gtx/euler_angles.hpp>

namespace ForgeSim::Sandbox
{
    // Shared translation, world rotation delta, and local scale ratios.
    // Rotation and scale use each member's own authored pivot, avoiding shear.
    [[nodiscard]] inline std::optional<ObjectBatchEdit> BuildObjectTransformEdits(
        const SandboxScene& scene, const SceneSelection& selection,
        const Transform& activeBefore, const Transform& activeAfter, GizmoOperation operation)
    {
        ObjectBatchEdit batch;
        const auto rotation = [](const Transform& t)
        { return glm::eulerAngleXYZ(t.rotationRadians.x, t.rotationRadians.y, t.rotationRadians.z); };
        const auto rotationDelta = rotation(activeAfter) * glm::transpose(rotation(activeBefore));
        for (const auto id : selection.SelectedObjectIds())
        {
            const auto* object = scene.FindObject(id);
            if (!object) return std::nullopt;
            auto after = object->transform;
            switch (operation)
            {
            case GizmoOperation::Translate:
                after.position += activeAfter.position - activeBefore.position;
                break;
            case GizmoOperation::Rotate:
            {
                auto matrix = rotationDelta * after.GizmoMatrix();
                matrix[3] = glm::vec4{after.position, 1.0f};
                if (!TryApplyGizmoTransform(after, matrix, operation)) return std::nullopt;
                break;
            }
            case GizmoOperation::Scale:
                for (int axis = 0; axis < 3; ++axis)
                {
                    if (std::abs(activeBefore.scale[axis]) < 1e-6f) return std::nullopt;
                    after.scale[axis] *= activeAfter.scale[axis] / activeBefore.scale[axis];
                }
                break;
            default: return std::nullopt;
            }
            for (int axis = 0; axis < 3; ++axis)
                if (!std::isfinite(after.position[axis]) || !std::isfinite(after.rotationRadians[axis]) ||
                    !std::isfinite(after.scale[axis]) || std::abs(after.scale[axis]) < 1e-6f) return std::nullopt;
            batch.edits.emplace_back(ObjectTransformEdit{id, object->transform, after});
        }
        return batch;
    }
}
