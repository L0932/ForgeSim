#pragma once
#include "Transform.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <cstddef>
#include <cmath>
#include <initializer_list>

namespace ForgeSim::Sandbox
{
    // Post-multiplied local adjustments; the shared imported asset stays immutable.
    // MSVC's std::map move constructor may allocate a sentinel. A sorted vector
    // keeps small sparse override sets deterministic and makes ownership moves
    // allocation-free, as required by entity restoration and undo history.
    class NodeTransformOverrides final
    {
        using Entries = std::vector<std::pair<std::size_t, Transform>>;
        Entries m_Entries;
    public:
        using iterator = Entries::iterator;
        using const_iterator = Entries::const_iterator;
        [[nodiscard]] iterator begin() noexcept { return m_Entries.begin(); }
        [[nodiscard]] iterator end() noexcept { return m_Entries.end(); }
        [[nodiscard]] const_iterator begin() const noexcept { return m_Entries.begin(); }
        [[nodiscard]] const_iterator end() const noexcept { return m_Entries.end(); }
        [[nodiscard]] bool empty() const noexcept { return m_Entries.empty(); }
        [[nodiscard]] std::size_t size() const noexcept { return m_Entries.size(); }
        void clear() noexcept { m_Entries.clear(); }
        [[nodiscard]] iterator find(std::size_t key) noexcept
        {
            const auto found = LowerBound(key);
            return found != end() && found->first == key ? found : end();
        }
        [[nodiscard]] const_iterator find(std::size_t key) const noexcept
        {
            const auto found = LowerBound(key);
            return found != end() && found->first == key ? found : end();
        }
        [[nodiscard]] bool contains(std::size_t key) const noexcept { return find(key) != end(); }
        std::pair<iterator, bool> emplace(std::size_t key, const Transform& value)
        {
            const auto found = LowerBound(key);
            if (found != end() && found->first == key) return {found, false};
            return {m_Entries.emplace(found, key, value), true};
        }
        std::pair<iterator, bool> insert_or_assign(std::size_t key, const Transform& value)
        {
            auto result = emplace(key, value);
            if (!result.second) result.first->second = value;
            return result;
        }
        Transform& operator[](std::size_t key) { return emplace(key, Transform{}).first->second; }
        Transform& at(std::size_t key)
        {
            const auto found = find(key);
            if (found == end()) throw std::out_of_range("Node override is unavailable.");
            return found->second;
        }
        const Transform& at(std::size_t key) const
        {
            const auto found = find(key);
            if (found == end()) throw std::out_of_range("Node override is unavailable.");
            return found->second;
        }
        std::size_t erase(std::size_t key)
        {
            const auto found = find(key);
            if (found == end()) return 0;
            m_Entries.erase(found);
            return 1;
        }
    private:
        iterator LowerBound(std::size_t key) noexcept
        {
            return std::lower_bound(begin(), end(), key,
                [](const auto& entry, std::size_t index) { return entry.first < index; });
        }
        const_iterator LowerBound(std::size_t key) const noexcept
        {
            return std::lower_bound(begin(), end(), key,
                [](const auto& entry, std::size_t index) { return entry.first < index; });
        }
    };
    static_assert(std::is_nothrow_move_constructible_v<NodeTransformOverrides>);
    static_assert(std::is_nothrow_move_assignable_v<NodeTransformOverrides>);
    [[nodiscard]] inline glm::mat4 NodeOverrideMatrix(const NodeTransformOverrides& overrides, std::size_t index)
    {
        const auto found = overrides.find(index);
        return found == overrides.end() ? glm::mat4{1} : found->second.ModelMatrix();
    }
    [[nodiscard]] inline bool ValidNodeOverride(const Transform& transform)
    {
        for (const auto& v : {transform.position, transform.rotationRadians, transform.scale, transform.pivot})
            for (int axis=0; axis<3; ++axis) if (!std::isfinite(v[axis])) return false;
        for (int axis=0; axis<3; ++axis) if (transform.scale[axis]==0) return false;
        const auto matrix=transform.ModelMatrix();
        for (int c=0;c<4;++c) for(int r=0;r<4;++r) if(!std::isfinite(matrix[c][r])) return false;
        return true;
    }
}
