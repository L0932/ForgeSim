#pragma once

#include "ConsoleCommands.hpp"

#include <algorithm>
#include <deque>
#include <string>
#include <string_view>
#include <utility>
#include <imgui.h>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Build/Version.hpp>

namespace ForgeSim::Sandbox
{
    class EditorConsole final
    {
    public:
        [[nodiscard]] bool WantsKeyboard() const noexcept { return m_WantsKeyboard; }

        void RegisterCommand(std::string name, std::string description, std::string usage,
            std::size_t minimumArguments, std::size_t maximumArguments, ConsoleCommands::Handler handler)
        {
            m_Commands.Register(std::move(name), std::move(description), std::move(usage),
                minimumArguments, maximumArguments, std::move(handler));
        }

        void Draw(std::string_view sceneName, std::string_view scenePath, bool dirty)
        {
            m_WantsKeyboard = false;
            const auto incoming = Core::ReadLogEntries(m_LastSequence);
            for (const auto& entry : incoming)
            {
                Append({entry, false});
                m_LastSequence = entry.sequence;
            }

            // Preserve the old Scene File window ID and its saved dock placement.
            if (ImGui::Begin("Console###Scene File"))
            {
                const std::string title = std::string(sceneName) + (dirty ? " *" : "");
                ImGui::TextWrapped("%s", title.c_str());
                ImGui::SetItemTooltip("%.*s", static_cast<int>(scenePath.size()),
                    scenePath.empty() ? "" : scenePath.data());
                // Keep the toolbar compact without changing other editor controls.
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
                if (ImGui::Button("Clear")) Clear();
                const auto option = [](const char* label, bool& value)
                {
                    const auto& style = ImGui::GetStyle();
                    const float width = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
                    const float used = ImGui::GetItemRectMax().x - ImGui::GetCursorScreenPos().x;
                    if (ImGui::GetContentRegionAvail().x - used >= width + style.ItemSpacing.x)
                        ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button,
                        style.Colors[value ? ImGuiCol_ButtonActive : ImGuiCol_Button]);
                    ImGui::PushStyleColor(ImGuiCol_Text,
                        style.Colors[value ? ImGuiCol_Text : ImGuiCol_TextDisabled]);
                    if (ImGui::Button(label)) value = !value;
                    ImGui::PopStyleColor(2);
                    ImGui::SetItemTooltip("%s: %s. Click to %s.", label,
                        value ? "on" : "off", value ? "disable" : "enable");
                };
                option("Auto-scroll", m_AutoScroll);
                option("Info", m_ShowInfo);
                option("Warnings", m_ShowWarnings);
                option("Errors", m_ShowErrors);
                ImGui::PopStyleVar(2);
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputTextWithHint("##ConsoleFilter", "Filter messages...",
                    m_Filter.InputBuf, IM_ARRAYSIZE(m_Filter.InputBuf))) m_Filter.Build();
                m_WantsKeyboard = ImGui::IsItemActive();

                if (ImGui::BeginChild("##ConsoleMessages", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None))
                {
                    const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
                    bool visible = false;
                    const auto& background = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                    const bool light = background.x + background.y + background.z > 1.5f;
                    for (const auto& row : m_Entries)
                    {
                        const auto& entry = row.log;
                        if ((!row.command && ((entry.level == Core::LogLevel::Info && !m_ShowInfo) ||
                            (entry.level == Core::LogLevel::Warning && !m_ShowWarnings) ||
                            (entry.level == Core::LogLevel::Error && !m_ShowErrors))) ||
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
                        if (row.command)
                            ImGui::TextWrapped("%s", entry.message.c_str());
                        else
                            ImGui::TextWrapped("[%s] %s", level, entry.message.c_str());
                        ImGui::PopStyleColor();
                        if (row.command)
                            ImGui::SetItemTooltip("Submitted command");
                        else
                            ImGui::SetItemTooltip("%s:%u", entry.sourceFile.c_str(),
                                static_cast<unsigned int>(entry.sourceLine));
                    }
                    if (!visible) ImGui::TextDisabled("No matching messages.");
                    if (m_ScrollToBottom || (m_AutoScroll && !incoming.empty() && atBottom))
                        ImGui::SetScrollHereY(1.0f);
                    m_ScrollToBottom = false;
                }
                ImGui::EndChild();
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled(">");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1.0f);
                if (m_RefocusInput)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_RefocusInput = false;
                }
                const bool submitted = ImGui::InputTextWithHint("##ConsoleCommand", "Enter command (help)",
                    m_Input, IM_ARRAYSIZE(m_Input), ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_CallbackHistory, HistoryCallback, this);
                m_WantsKeyboard = m_WantsKeyboard || ImGui::IsItemActive() || submitted;
                if (submitted)
                {
                    std::string line(m_Input);
                    const auto first = line.find_first_not_of(" \t\r\n");
                    if (first != std::string::npos)
                    {
                        line = line.substr(first, line.find_last_not_of(" \t\r\n") - first + 1);
                        m_History.Add(line);
                        AppendCommand(line);
                        for (const auto& result : m_Commands.ExecuteLine(line,
                            {sceneName, scenePath, dirty, Build::Version}))
                        {
                            if (result.clear) Clear();
                            if (!result.message.empty())
                                Core::Log(result.error ? Core::LogLevel::Error : Core::LogLevel::Info,
                                    result.message);
                        }
                        m_ScrollToBottom = true;
                    }
                    m_Input[0] = '\0';
                    m_RefocusInput = true;
                }
            }
            ImGui::End();
        }

    private:
        struct Row
        {
            Core::LogEntry log;
            bool command = false;
        };

        void Append(Row row)
        {
            m_Entries.push_back(std::move(row));
            if (m_Entries.size() > Core::LogHistoryCapacity) m_Entries.pop_front();
        }

        void AppendCommand(std::string_view line)
        {
            // Keep application messages preceding this command in order.
            for (const auto& entry : Core::ReadLogEntries(m_LastSequence))
            {
                Append({entry, false});
                m_LastSequence = entry.sequence;
            }
            Core::LogEntry echo;
            echo.message = "> " + std::string(line);
            Append({std::move(echo), true});
        }

        void Clear()
        {
            // Consume any pending echo/results so clear cannot reappear next frame.
            const auto pending = Core::ReadLogEntries(m_LastSequence);
            if (!pending.empty()) m_LastSequence = pending.back().sequence;
            m_Entries.clear();
        }

        static int HistoryCallback(ImGuiInputTextCallbackData* data)
        {
            auto& console = *static_cast<EditorConsole*>(data->UserData);
            const auto line = data->EventKey == ImGuiKey_UpArrow
                ? console.m_History.Previous(data->Buf) : console.m_History.Next();
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, line.c_str());
            return 0;
        }

        ConsoleCommands m_Commands;
        ConsoleCommandHistory m_History;
        char m_Input[1024]{};
        bool m_WantsKeyboard = false;
        bool m_RefocusInput = false;
        bool m_ScrollToBottom = false;
        std::uint64_t m_LastSequence = 0;
        std::deque<Row> m_Entries;
        ImGuiTextFilter m_Filter;
        bool m_AutoScroll = true;
        bool m_ShowInfo = true;
        bool m_ShowWarnings = true;
        bool m_ShowErrors = true;
    };
}
