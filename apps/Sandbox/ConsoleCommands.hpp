#pragma once

#include <cctype>
#include <exception>
#include <functional>
#include <map>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ForgeSim::Sandbox
{
    struct ConsoleCommandContext
    {
        std::string_view sceneName;
        std::string_view scenePath;
        bool dirty = false;
        std::string_view version;
    };

    struct ConsoleCommandResult
    {
        std::string message;
        bool error = false;
        bool clear = false;
    };

    // No ImGui or graphics dependency. Commands execute on the calling thread.
    class ConsoleCommands final
    {
    public:
        using Handler = std::function<ConsoleCommandResult(
            std::span<const std::string>, const ConsoleCommandContext&)>;

        ConsoleCommands()
        {
            Register("clear", "Clear the Console display.", "clear", 0, 0,
                [](auto, const auto&) { return ConsoleCommandResult{"", false, true}; });
            Register("scene.info", "Show the current scene and save state.", "scene.info", 0, 0,
                [](auto, const ConsoleCommandContext& context)
                {
                    return ConsoleCommandResult{"Scene: " + std::string(context.sceneName) +
                        "\nPath: " + (context.scenePath.empty() ? std::string("Not saved") :
                            std::string(context.scenePath)) +
                        "\nUnsaved changes: " + (context.dirty ? "yes" : "no")};
                });
            Register("version", "Show the ForgeSim build version.", "version", 0, 0,
                [](auto, const ConsoleCommandContext& context)
                { return ConsoleCommandResult{"ForgeSim " + std::string(context.version)}; });
        }

        void Register(std::string name, std::string description, std::string usage,
            std::size_t minimumArguments, std::size_t maximumArguments, Handler handler)
        {
            if (name.empty() || name == "help" || !handler ||
                minimumArguments > maximumArguments ||
                name.find_first_of(" \t\r\n") != std::string::npos)
                throw std::invalid_argument("Invalid Console command registration.");
            if (!m_Commands.emplace(std::move(name), Command{std::move(description),
                std::move(usage), minimumArguments, maximumArguments, std::move(handler)}).second)
                throw std::invalid_argument("Duplicate Console command registration.");
        }

        // Validate quote boundaries before executing any part of a chain.
        // Runtime errors stop the chain; earlier successful commands remain applied.
        [[nodiscard]] std::vector<ConsoleCommandResult> ExecuteLine(std::string_view line,
            const ConsoleCommandContext& context = {}) const
        {
            std::vector<std::string_view> commands;
            bool quoted = false;
            std::size_t start = 0;
            for (std::size_t index = 0; index < line.size(); ++index)
            {
                if (line[index] == '"') quoted = !quoted;
                else if (line[index] == ';' && !quoted)
                {
                    commands.push_back(line.substr(start, index - start));
                    start = index + 1;
                }
            }
            if (quoted) return {{"Unclosed double quote in command line. No commands executed.", true}};
            commands.push_back(line.substr(start));
            std::vector<ConsoleCommandResult> results;
            for (auto command : commands)
            {
                if (command.find_first_not_of(" \t\r\n") == std::string_view::npos) continue;
                results.push_back(Execute(command, context));
                if (results.back().error) break;
            }
            return results;
        }

        [[nodiscard]] ConsoleCommandResult Execute(std::string_view line,
            const ConsoleCommandContext& context = {}) const
        {
            try
            {
                std::vector<std::string> words;
                // Double quotes group arguments; backslashes remain literal so
                // native Windows paths, including trailing separators, work.
                std::string word;
                bool quoted = false;
                bool started = false;
                for (char character : line)
                {
                    if (character == '"') { quoted = !quoted; started = true; }
                    else if (!quoted && std::isspace(static_cast<unsigned char>(character)))
                    {
                        if (started) { words.push_back(std::move(word)); word.clear(); started = false; }
                    }
                    else { word += character; started = true; }
                }
                if (quoted) return {"Unclosed double quote in command.", true};
                if (started) words.push_back(std::move(word));
                if (words.empty()) return {};
                if (words[0] == "help")
                {
                    if (words.size() > 2) return {"Usage: help [command]", true};
                    if (words.size() == 2)
                    {
                        if (words[1] == "help") return {"help [command] - List commands or show command usage."};
                        const auto found = m_Commands.find(words[1]);
                        if (found == m_Commands.end()) return {"Unknown command: " + words[1], true};
                        return {found->second.usage + " - " + found->second.description};
                    }
                    std::string result = "help [command] - List commands or show command usage.";
                    for (const auto& [name, command] : m_Commands)
                    {
                        static_cast<void>(name);
                        result += "\n" + command.usage + " - " + command.description;
                    }
                    return {std::move(result)};
                }
                const auto found = m_Commands.find(words[0]);
                if (found == m_Commands.end())
                    return {"Unknown command: " + words[0] + ". Type help for available commands.", true};
                const auto& command = found->second;
                const std::span<const std::string> arguments(words.data() + 1, words.size() - 1);
                if (arguments.size() < command.minimumArguments || arguments.size() > command.maximumArguments)
                    return {"Usage: " + command.usage, true};
                return command.handler(arguments, context);
            }
            catch (const std::exception& exception)
            { return {"Command failed: " + std::string(exception.what()), true}; }
            catch (...)
            { return {"Command failed: unknown execution error.", true}; }
        }

    private:
        struct Command
        {
            std::string description;
            std::string usage;
            std::size_t minimumArguments;
            std::size_t maximumArguments;
            Handler handler;
        };
        std::map<std::string, Command, std::less<>> m_Commands;
    };

    class ConsoleCommandHistory final
    {
    public:
        void Add(std::string_view line)
        {
            if (!line.empty() && (m_Entries.empty() || m_Entries.back() != line))
            {
                m_Entries.emplace_back(line);
                if (m_Entries.size() > 100) m_Entries.erase(m_Entries.begin());
            }
            m_Position = m_Entries.size();
            m_Draft.clear();
        }

        [[nodiscard]] std::string Previous(std::string_view draft)
        {
            if (m_Position == m_Entries.size()) m_Draft = draft;
            if (m_Position > 0) --m_Position;
            return m_Position < m_Entries.size() ? m_Entries[m_Position] : m_Draft;
        }

        [[nodiscard]] std::string Next()
        {
            if (m_Position < m_Entries.size()) ++m_Position;
            return m_Position < m_Entries.size() ? m_Entries[m_Position] : m_Draft;
        }

    private:
        std::vector<std::string> m_Entries;
        std::size_t m_Position = 0;
        std::string m_Draft;
    };
}
