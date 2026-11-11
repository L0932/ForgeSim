#pragma once

#include <algorithm>
#include <array>
#include <cstdio>

#include <imgui.h>

namespace ForgeSim::Sandbox::InspectorUI
{
    // Position the label and control without introducing different ID scopes
    // at responsive breakpoints. The same field keeps its ID in both layouts.
    template<typename DrawControl>
    bool Property(const char* id, const char* label, DrawControl drawControl)
    {
        ImGui::PushID(id);
        ImGui::BeginGroup();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float available = ImGui::GetContentRegionAvail().x;
        const auto& style = ImGui::GetStyle();
        const float labelWidth = ImGui::GetFontSize() * 8.5f;
        const bool inlineRow = available >= labelWidth + style.ItemSpacing.x
            + ImGui::GetFontSize() * 14.0f;

        if (inlineRow)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + labelWidth);
            ImGui::TextUnformatted(label);
            ImGui::PopTextWrapPos();
        }
        else
        {
            ImGui::TextWrapped("%s", label);
        }
        const float labelBottom = ImGui::GetItemRectMax().y;
        if (inlineRow)
            ImGui::SetCursorScreenPos(ImVec2(origin.x + labelWidth + style.ItemSpacing.x, origin.y));

        ImGui::SetNextItemWidth(-1.0f);
        const bool changed = drawControl();

        if (inlineRow)
        {
            const float nextY = std::max(ImGui::GetCursorScreenPos().y,
                labelBottom + style.ItemSpacing.y);
            ImGui::SetCursorScreenPos(ImVec2(origin.x, nextY - style.ItemSpacing.y));
            ImGui::Dummy(ImVec2(0.0f, 0.0f));
        }
        ImGui::EndGroup();
        ImGui::PopID();
        return changed;
    }

    inline bool DragFloat(const char* id, const char* label, float* value,
        float speed = 1.0f, float minimum = 0.0f, float maximum = 0.0f,
        const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        return Property(id, label, [&]
        {
            return ImGui::DragFloat("##value", value, speed, minimum, maximum, format, flags);
        });
    }

    inline bool ColorEdit3(const char* id, const char* label, float* value,
        ImGuiColorEditFlags flags = 0)
    {
        return Property(id, label, [&]
        {
            return ImGui::ColorEdit3("##value", value, flags);
        });
    }

    inline bool DragFloat3(const char* id, const char* label, float* value,
        float speed = 1.0f, float minimum = 0.0f, float maximum = 0.0f,
        const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        return Property(id, label, [&]
        {
            const auto& style = ImGui::GetStyle();
            constexpr std::array<const char*, 3> axes{"X", "Y", "Z"};
            const float available = ImGui::GetContentRegionAvail().x;
            const float axisWidth = ImGui::CalcTextSize("X").x;
            float numberWidth = ImGui::GetFontSize() * 3.0f;
            for (int axis = 0; axis < 3; ++axis)
            {
                std::array<char, 128> text{};
                std::snprintf(text.data(), text.size(), format, static_cast<double>(value[axis]));
                numberWidth = std::max(numberWidth, ImGui::CalcTextSize(text.data()).x);
            }
            const float minimumSlot = axisWidth + style.ItemInnerSpacing.x
                + numberWidth + 2.0f * style.FramePadding.x;
            const bool horizontal = available >= minimumSlot * 3.0f + style.ItemSpacing.x * 2.0f;
            const float slotWidth = horizontal
                ? (available - style.ItemSpacing.x * 2.0f) / 3.0f : available;
            bool changed = false;
            for (int axis = 0; axis < 3; ++axis)
            {
                if (axis != 0 && horizontal) ImGui::SameLine();
                ImGui::PushID(axis);
                ImGui::BeginGroup();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(axes[axis]);
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::SetNextItemWidth(std::max(1.0f,
                    slotWidth - axisWidth - style.ItemInnerSpacing.x));
                changed |= ImGui::DragFloat("##value", &value[axis], speed,
                    minimum, maximum, format, flags);
                ImGui::EndGroup();
                ImGui::PopID();
            }
            return changed;
        });
    }
}
