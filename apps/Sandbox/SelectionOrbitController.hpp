#pragma once

#include "SceneCameraFraming.hpp"
#include "SceneSelection.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <glm/gtc/constants.hpp>

namespace ForgeSim::Sandbox
{
    // Live camera navigation only. Stores no scene references or authored edits.
    class SelectionOrbitController final
    {
    public:
        [[nodiscard]] bool Active() const noexcept { return m_Target.has_value(); }
        void Stop() noexcept { m_Target.reset(); }
        void ObserveSelection(const SceneSelection& selection)
        {
            if (!m_Target) return;
            const auto& first = m_Target->SelectedNode(); const auto& second = selection.SelectedNode();
            bool sameNode = first.has_value() == second.has_value();
            if (first && second)
                sameNode = first->objectId == second->objectId && first->nodeIndex == second->nodeIndex &&
                    first->modelReference == second->modelReference;
            const auto a = m_Target->SelectedObjectIds(); const auto b = selection.SelectedObjectIds();
            if (!sameNode || m_Target->SelectedLightId() != selection.SelectedLightId() ||
                a.size() != b.size() || !std::is_permutation(a.begin(),a.end(),b.begin(),b.end())) Stop();
        }
        [[nodiscard]] Renderer::PerspectiveCamera Start(const SceneSelection& selection,
            const Renderer::AxisAlignedBoundingBox& bounds, const Renderer::AxisAlignedBoundingBox& sceneBounds,
            const Renderer::PerspectiveCamera& current, float aspectRatio)
        {
            if (selection.Empty()) throw std::invalid_argument("Orbit requires a selection.");
            const auto framed = CreateSelectionFocusCamera(bounds,sceneBounds,current,aspectRatio);
            const auto center = bounds.minimum*.5f + bounds.maximum*.5f;
            const auto offset = framed.Position()-center;
            const float distance = glm::length(offset);
            m_Distance = std::max(distance, .0001f);
            m_Yaw = std::atan2(offset.x,offset.z);
            m_Pitch = std::clamp(std::asin(std::clamp(offset.y/m_Distance,-1.0f,1.0f)), -PitchLimit, PitchLimit);
            const auto result = MakeCamera(bounds,sceneBounds,framed,m_Yaw,m_Pitch,m_Distance);
            m_Target = selection;
            return result;
        }
        [[nodiscard]] Renderer::PerspectiveCamera Update(
            const Renderer::AxisAlignedBoundingBox& bounds, const Renderer::AxisAlignedBoundingBox& sceneBounds,
            const Renderer::PerspectiveCamera& current, double mouseX, double mouseY, double wheel)
        {
            if (!Active() || !std::isfinite(mouseX) || !std::isfinite(mouseY) || !std::isfinite(wheel)) return current;
            for (const auto& box : {bounds,sceneBounds})
                for (int axis=0;axis<3;++axis)
                    if (!std::isfinite(box.minimum[axis]) || !std::isfinite(box.maximum[axis]) ||
                        box.minimum[axis]>box.maximum[axis]) return current;
            const float yaw = std::remainder(m_Yaw - static_cast<float>(std::clamp(mouseX,-10000.0,10000.0))*.005f,
                glm::two_pi<float>());
            const float pitch = std::clamp(m_Pitch + static_cast<float>(std::clamp(mouseY,-10000.0,10000.0))*.005f,
                -PitchLimit,PitchLimit);
            const double radius = glm::length(glm::dvec3{bounds.maximum}*.5-glm::dvec3{bounds.minimum}*.5);
            const double minimum = std::clamp(radius*.02,.0001,1e9);
            const double maximum = std::max(minimum*2,std::min(std::max(radius*1000,static_cast<double>(m_Distance)*2),1e12));
            const float distance = static_cast<float>(std::clamp(static_cast<double>(m_Distance)*
                std::exp(-std::clamp(wheel,-128.0,128.0)*.15),minimum,maximum));
            const auto result = MakeCamera(bounds,sceneBounds,current,yaw,pitch,distance);
            m_Yaw=yaw; m_Pitch=pitch; m_Distance=distance;
            return result;
        }
    private:
        static constexpr float PitchLimit = glm::half_pi<float>() - .01f;
        static Renderer::PerspectiveCamera MakeCamera(const Renderer::AxisAlignedBoundingBox& bounds,
            const Renderer::AxisAlignedBoundingBox& sceneBounds, const Renderer::PerspectiveCamera& current,
            float yaw, float pitch, float distance)
        {
            const glm::vec3 center = bounds.minimum*.5f+bounds.maximum*.5f;
            const glm::vec3 direction{std::cos(pitch)*std::sin(yaw),std::sin(pitch),std::cos(pitch)*std::cos(yaw)};
            const auto position = center+direction*distance;
            const float nearPlane=std::min(current.NearClippingPlane(),distance*.1f);
            float farPlane=std::max(current.FarClippingPlane(),nearPlane*2);
            for(const auto& box : {bounds,sceneBounds})
                for(unsigned int corner=0;corner<8;++corner)
                {
                    const glm::vec3 p{corner&1u?box.maximum.x:box.minimum.x,corner&2u?box.maximum.y:box.minimum.y,
                        corner&4u?box.maximum.z:box.minimum.z};
                    farPlane=std::max(farPlane,glm::length(p-position)*1.1f);
                }
            return {position,center,{0,1,0},current.VerticalFieldOfViewRadians(),nearPlane,farPlane};
        }
        std::optional<SceneSelection> m_Target;
        float m_Yaw=0, m_Pitch=0, m_Distance=1;
    };
}
