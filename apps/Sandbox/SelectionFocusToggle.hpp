#pragma once

#include "SceneCameraFraming.hpp"
#include "SceneCameraState.hpp"
#include "SceneSelection.hpp"
#include <algorithm>
#include <cmath>

namespace ForgeSim::Sandbox
{
    // Temporary navigation state; never stored in scene documents or edit history.
    class SelectionFocusToggle final
    {
    public:
        void Reset() noexcept
        {
            m_Target.reset();
            m_Previous.reset();
            m_Focused.reset();
            m_Bounds.reset();
        }
        [[nodiscard]] bool HasReturnView(const Renderer::PerspectiveCamera& current) const
        {
            return m_Previous && m_Focused && MatchesView(CaptureSceneCamera(current), *m_Focused);
        }
        // Call after free-camera updates. Clicking/capturing the viewport alone
        // must not end focus or discard the return view.
        [[nodiscard]] Renderer::PerspectiveCamera ObserveNavigation(
            const Renderer::PerspectiveCamera& current)
        {
            if (!m_Previous || !m_Focused || MatchesView(CaptureSceneCamera(current), *m_Focused))
                return current;
            auto state = CaptureSceneCamera(current);
            RestoreTemporaryClipping(state);
            const auto result = CreateSceneCamera(state);
            Reset();
            return result;
        }
        void ObserveSelection(const SceneSelection& selection)
        {
            const auto target = Identify(selection);
            if (!m_Target || *m_Target != target)
            {
                Reset();
                m_Target = target;
            }
        }
        [[nodiscard]] Renderer::PerspectiveCamera Toggle(const SceneSelection& selection,
            const Renderer::AxisAlignedBoundingBox& bounds,
            const Renderer::AxisAlignedBoundingBox& sceneBounds,
            const Renderer::PerspectiveCamera& current, float aspectRatio)
        {
            ObserveSelection(selection);
            const auto pose = CaptureSceneCamera(current);
            if (m_Previous && m_Focused && m_Bounds && MatchesView(pose, *m_Focused) &&
                Close(bounds.minimum, m_Bounds->minimum) && Close(bounds.maximum, m_Bounds->maximum))
            {
                auto previous = *m_Previous;
                // Restore automatic clipping changes, retaining Inspector edits.
                previous.nearClippingPlane = pose.nearClippingPlane;
                previous.farClippingPlane = pose.farClippingPlane;
                RestoreTemporaryClipping(previous);
                const auto restored = CreateSceneCamera(previous);
                m_Previous.reset(); m_Focused.reset(); m_Bounds.reset();
                return restored;
            }
            // Navigation, external camera edits, or moved geometry start a fresh pair.
            const auto focused = CreateSelectionFocusCamera(bounds, sceneBounds, current, aspectRatio);
            m_Previous = pose;
            m_Focused = CaptureSceneCamera(focused);
            m_Bounds = bounds;
            return focused;
        }

        [[nodiscard]] Renderer::PerspectiveCamera EndForNavigation(
            const Renderer::PerspectiveCamera& current)
        {
            auto state = CaptureSceneCamera(current);

            if (m_Previous && m_Focused)
            {
                // Preserve clipping values explicitly edited after focusing.
                if (Close(state.nearClippingPlane,
                    m_Focused->nearClippingPlane))
                {
                    state.nearClippingPlane =
                        m_Previous->nearClippingPlane;
                }

                if (Close(state.farClippingPlane,
                    m_Focused->farClippingPlane))
                {
                    state.farClippingPlane =
                        m_Previous->farClippingPlane;
                }
            }

            Reset();
            return CreateSceneCamera(state);
        }

    private:
        struct Target
        {
            std::vector<SandboxObjectId> objects;
            std::optional<SceneLightId> light;
            std::optional<SandboxObjectId> nodeOwner;
            std::optional<std::size_t> nodeIndex;
            std::string modelReference;
            bool operator==(const Target&) const = default;
        };
        static Target Identify(const SceneSelection& selection)
        {
            Target target;
            const auto ids = selection.SelectedObjectIds();
            target.objects.assign(ids.begin(), ids.end());
            std::sort(target.objects.begin(), target.objects.end());
            target.light = selection.SelectedLightId();
            if (const auto& node = selection.SelectedNode())
            {
                target.nodeOwner = node->objectId; target.nodeIndex = node->nodeIndex;
                target.modelReference = node->modelReference;
            }
            return target;
        }
        static bool Close(float a, float b)
        { return std::abs(a - b) <= 1e-5f * std::max({1.0f, std::abs(a), std::abs(b)}); }
        static bool Close(glm::vec3 a, glm::vec3 b)
        { return Close(a.x,b.x) && Close(a.y,b.y) && Close(a.z,b.z); }
        void RestoreTemporaryClipping(SceneCameraData& state) const
        {
            // Compare each field exactly: even small explicit clipping edits
            // should survive. Copying camera state preserves these float values.
            const float nearPlane = state.nearClippingPlane == m_Focused->nearClippingPlane
                ? m_Previous->nearClippingPlane : state.nearClippingPlane;
            const float farPlane = state.farClippingPlane == m_Focused->farClippingPlane
                ? m_Previous->farClippingPlane : state.farClippingPlane;
            // A mixed automatic/manual pair must still form a valid frustum.
            if (nearPlane > 0.0f && farPlane > nearPlane)
            {
                state.nearClippingPlane = nearPlane;
                state.farClippingPlane = farPlane;
            }
        }
        static bool MatchesView(const SceneCameraData& a, const SceneCameraData& b)
        {
            return Close(a.position,b.position) && Close(a.forwardDirection,b.forwardDirection) &&
                Close(a.upDirection,b.upDirection) && Close(a.verticalFieldOfViewRadians,b.verticalFieldOfViewRadians);
        }
        std::optional<Target> m_Target;
        std::optional<SceneCameraData> m_Previous, m_Focused;
        std::optional<Renderer::AxisAlignedBoundingBox> m_Bounds;
    };
}
