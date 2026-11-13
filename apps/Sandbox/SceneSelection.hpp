#pragma once

#include "SandboxObject.hpp"
#include "SceneLight.hpp"
#include <algorithm>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace ForgeSim::Sandbox
{
    struct ModelNodeHit { std::size_t nodeIndex; float distance; };

    struct ModelNodeSelection
    {
        SandboxObjectId objectId;
        std::size_t nodeIndex;
        std::string modelReference;
    };

    // Objects form an ordered set. The last selected object is the active target.
    // Lights retain single selection until mixed object/light editing is supported.
    class SceneSelection final
    {
    public:
        void SelectObject(SandboxObjectId id)
        {
            m_Node.reset();
            m_Objects.assign(1, id);
            m_Light.reset();
            m_Anchor = id;
        }
        void AddObject(SandboxObjectId id)
        {
            m_Node.reset();
            m_Light.reset();
            std::erase(m_Objects, id);
            m_Objects.push_back(id);
            m_Anchor = id;
        }
        void RemoveObject(SandboxObjectId id) noexcept
        {
            if (m_Node && m_Node->objectId == id) m_Node.reset();
            std::erase(m_Objects, id);
            if (m_Anchor == id) m_Anchor = SelectedObjectId();
        }
        void ClickObject(SandboxObjectId id, bool ctrl, bool shift,
            std::span<const SandboxObjectId> rangeOrder = {})
        {
            const auto anchor = m_Anchor;
            const auto first = anchor ? std::find(rangeOrder.begin(), rangeOrder.end(), *anchor) : rangeOrder.end();
            const auto last = std::find(rangeOrder.begin(), rangeOrder.end(), id);
            if (shift && first != rangeOrder.end() && last != rangeOrder.end())
            {
                if (!ctrl) Clear();
                for (auto it = std::min(first, last); it <= std::max(first, last); ++it) AddObject(*it);
                AddObject(id);
                m_Anchor = anchor;
            }
            else if (ctrl && IsObjectSelected(id)) RemoveObject(id);
            else if (ctrl || shift) AddObject(id);
            else SelectObject(id);
        }
        void SelectNode(SandboxObjectId id, std::size_t index, std::string reference)
        {
            Clear();
            m_Node = ModelNodeSelection{id, index, std::move(reference)};
        }
        [[nodiscard]] const std::optional<ModelNodeSelection>& SelectedNode() const noexcept { return m_Node; }
        [[nodiscard]] bool IsNodeSelected(SandboxObjectId id, std::size_t index) const noexcept
        { return m_Node && m_Node->objectId == id && m_Node->nodeIndex == index; }
        void SelectLight(SceneLightId id) noexcept
        {
            Clear();
            if (id != InvalidSceneLightId) m_Light = id;
        }
        void Clear() noexcept { m_Objects.clear(); m_Light.reset(); m_Anchor.reset(); m_Node.reset(); }
        [[nodiscard]] bool Empty() const noexcept { return m_Objects.empty() && !m_Light && !m_Node; }
        [[nodiscard]] std::optional<SandboxObjectId> SelectedObjectId() const noexcept
        { return m_Objects.empty() ? std::nullopt : std::optional{m_Objects.back()}; }
        [[nodiscard]] std::span<const SandboxObjectId> SelectedObjectIds() const noexcept { return m_Objects; }
        [[nodiscard]] std::optional<SceneLightId> SelectedLightId() const noexcept { return m_Light; }
        [[nodiscard]] bool IsObjectSelected(SandboxObjectId id) const noexcept
        { return std::find(m_Objects.begin(), m_Objects.end(), id) != m_Objects.end(); }
        [[nodiscard]] bool IsLightSelected(SceneLightId id) const noexcept { return m_Light == id; }
        template<class Predicate> void RemoveObjectsIf(Predicate predicate)
        {
            std::erase_if(m_Objects, predicate);
            if (m_Anchor && !IsObjectSelected(*m_Anchor)) m_Anchor = SelectedObjectId();
        }
    private:
        std::optional<ModelNodeSelection> m_Node;
        std::vector<SandboxObjectId> m_Objects;
        std::optional<SceneLightId> m_Light;
        std::optional<SandboxObjectId> m_Anchor;
    };
}
