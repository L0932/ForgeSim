#pragma once

#include "SceneAssetPath.hpp"
#include <cmath>
#include <array>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <optional>
#include <string>
#include <stdexcept>

namespace ForgeSim::Sandbox
{
    enum class HeightWrap { Repeat, MirroredRepeat, ClampToEdge };

    struct MaterialDisplacement
    {
        bool enabled = false;
        std::optional<std::size_t> imageIndex;
        std::string heightAsset; // Project-relative external PNG, or empty for embedded image.
        unsigned channel = 0;
        float scale = .050f;
        float midlevel = .225f;
        unsigned subdivisions = 1;
        bool inverted = false;
        bool stitchSeams = true;
        std::array<float,2> uvScale{1,1};
        std::array<float,2> uvOffset{0,0};
        float uvRotationRadians = 0;
        HeightWrap wrapU = HeightWrap::Repeat;
        HeightWrap wrapV = HeightWrap::Repeat;
        bool operator==(const MaterialDisplacement&) const = default;
    };
    // MSVC's map move constructor may allocate a sentinel. Scene restoration
    // and edit-history commits require allocation-free ownership moves.
    // Keep the small sparse material set sorted, as with NodeTransformOverrides.
    class MaterialDisplacements final
    {
        using Entries = std::vector<std::pair<std::size_t, MaterialDisplacement>>;
        static_assert(std::is_nothrow_move_constructible_v<Entries>);
        static_assert(std::is_nothrow_move_assignable_v<Entries>);
        Entries m_Entries;
    public:
        using iterator = Entries::iterator;
        using const_iterator = Entries::const_iterator;
        MaterialDisplacements() = default;
        MaterialDisplacements(std::initializer_list<Entries::value_type> values)
        { for (const auto& [key, value] : values) emplace(key,value); }
        MaterialDisplacements(const MaterialDisplacements&) = default;
        MaterialDisplacements& operator=(const MaterialDisplacements&) = default;
        MaterialDisplacements(MaterialDisplacements&&) noexcept = default;
        MaterialDisplacements& operator=(MaterialDisplacements&&) noexcept = default;
        bool operator==(const MaterialDisplacements&) const = default;
        [[nodiscard]] iterator begin() noexcept { return m_Entries.begin(); }
        [[nodiscard]] iterator end() noexcept { return m_Entries.end(); }
        [[nodiscard]] const_iterator begin() const noexcept { return m_Entries.begin(); }
        [[nodiscard]] const_iterator end() const noexcept { return m_Entries.end(); }
        [[nodiscard]] bool empty() const noexcept { return m_Entries.empty(); }
        [[nodiscard]] std::size_t size() const noexcept { return m_Entries.size(); }
        void clear() noexcept { m_Entries.clear(); }
        void swap(MaterialDisplacements& other) noexcept { m_Entries.swap(other.m_Entries); }
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
        std::pair<iterator,bool> emplace(std::size_t key, const MaterialDisplacement& value)
        {
            const auto found = LowerBound(key);
            if (found != end() && found->first == key) return {found,false};
            return {m_Entries.emplace(found,key,value),true};
        }
        std::pair<iterator,bool> insert_or_assign(std::size_t key, const MaterialDisplacement& value)
        {
            auto result = emplace(key,value);
            if (!result.second) result.first->second = value;
            return result;
        }
        MaterialDisplacement& operator[](std::size_t key)
        { return emplace(key,MaterialDisplacement{}).first->second; }
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
            return std::lower_bound(begin(),end(),key,
                [](const auto& entry, std::size_t index) { return entry.first < index; });
        }
        const_iterator LowerBound(std::size_t key) const noexcept
        {
            return std::lower_bound(begin(),end(),key,
                [](const auto& entry, std::size_t index) { return entry.first < index; });
        }
    };
    static_assert(std::is_nothrow_move_constructible_v<MaterialDisplacements>);
    static_assert(std::is_nothrow_move_assignable_v<MaterialDisplacements>);

    inline void ValidateMaterialDisplacement(const MaterialDisplacement& value)
    {
        const auto validWrap = [](HeightWrap wrap)
        {
            return wrap == HeightWrap::Repeat || wrap == HeightWrap::MirroredRepeat ||
                wrap == HeightWrap::ClampToEdge;
        };
        if (!std::isfinite(value.scale) || !std::isfinite(value.midlevel) || value.midlevel < 0 || value.midlevel > 1 ||
            value.subdivisions > 8 || value.channel > 3 ||
            !std::isfinite(value.uvScale[0]) || !std::isfinite(value.uvScale[1]) ||
            !std::isfinite(value.uvOffset[0]) || !std::isfinite(value.uvOffset[1]) ||
            !std::isfinite(value.uvRotationRadians) || !validWrap(value.wrapU) || !validWrap(value.wrapV))
            throw std::invalid_argument("Invalid height settings: finite scale, midlevel 0–1, channel 0–3, levels 0–8 required.");
        if ((!value.heightAsset.empty() && (!IsValidSceneAssetPath(value.heightAsset) || value.imageIndex)) ||
            (value.enabled && value.heightAsset.empty() && !value.imageIndex))
            throw std::invalid_argument("Choose exactly one embedded height image or a project-relative external PNG.");
    }

}
