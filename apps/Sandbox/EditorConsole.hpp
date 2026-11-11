#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <string_view>
#include <imgui.h>
#include <ForgeSim/Core/Log.hpp>

namespace ForgeSim::Sandbox
{
    class EditorConsole final
    {
    public:
        void Draw(std::string_view sceneName, std::string_view scenePath, bool dirty)
        {
            const auto incoming = Core::ReadLogEntries(m_LastSequence);
            for (const auto& entry : incoming)
            {
                m_Entries.push_back(entry);
                m_LastSequence = entry.sequence;
                if (m_Entries.size() > Core::LogHistoryCapacity) m_Entries.pop_front();
            }

            // Preserve the old Scene File window ID and its saved dock placement.
            if (ImGui::Begin("Console###Scene File"))
            {
                const std::string title = std::string(sceneName) + (dirty ? " *" : "");
                ImGui::TextWrapped("%s", title.c_str());
                ImGui::SetItemTooltip("%.*s", static_cast<int>(scenePath.size()),
                    scenePath.empty() ? "" : scenePath.data());
                if (ImGui::Button("Clear")) m_Entries.clear();
                const auto option = [](const char* label, bool& value)
                {
                    const float width = ImGui::GetFrameHeight() +
                        ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(label).x;
                    const float used = ImGui::GetItemRectMax().x - ImGui::GetCursorScreenPos().x;
                    if (ImGui::GetContentRegionAvail().x - used > width + ImGui::GetStyle().ItemSpacing.x)
                        ImGui::SameLine();
                    ImGui::Checkbox(label, &value);
                };
                option("Auto-scroll", m_AutoScroll);
                option("Info", m_ShowInfo);
                option("Warnings", m_ShowWarnings);
                option("Errors", m_ShowErrors);
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputTextWithHint("##ConsoleFilter", "Filter messages...",
                    m_Filter.InputBuf, IM_ARRAYSIZE(m_Filter.InputBuf))) m_Filter.Build();

                if (ImGui::BeginChild("##ConsoleMessages", ImVec2(0,0), ImGuiChildFlags_None))
                {
                    const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
                    bool visible = false;
                    const auto& background = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                    const bool light = background.x + background.y + background.z > 1.5f;
                    for (const auto& entry : m_Entries)
                    {
                        if ((entry.level == Core::LogLevel::Info && !m_ShowInfo) ||
                            (entry.level == Core::LogLevel::Warning && !m_ShowWarnings) ||
                            (entry.level == Core::LogLevel::Error && !m_ShowErrors) ||
                            !m_Filter.PassFilter(entry.message.c_str())) continue;
                        visible = true;
                        auto color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
                        const char* level = "Info";
                        if (entry.level == Core::LogLevel::Warning)
                        {
                            level = "Warning";
                            color = light ? ImVec4(.52f,.28f,.02f,1) : ImVec4(.97f,.75f,.30f,1);
                        }
                        if (entry.level == Core::LogLevel::Error)
                        {
                            level = "Error";
                            color = light ? ImVec4(.70f,.08f,.08f,1) : ImVec4(1,.40f,.37f,1);
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::TextWrapped("[%s] %s", level, entry.message.c_str());
                        ImGui::PopStyleColor();
                        ImGui::SetItemTooltip("%s:%u", entry.sourceFile.c_str(),
                            static_cast<unsigned int>(entry.sourceLine));
                    }
                    if (!visible) ImGui::TextDisabled("No matching messages.");
                    if (m_AutoScroll && !incoming.empty() && atBottom)
                        ImGui::SetScrollHereY(1.0f);
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }

    private:
        std::uint64_t m_LastSequence = 0;
        std::deque<Core::LogEntry> m_Entries;
        ImGuiTextFilter m_Filter;
        bool m_AutoScroll = true;
        bool m_ShowInfo = true;
        bool m_ShowWarnings = true;
        bool m_ShowErrors = true;
    };
}
