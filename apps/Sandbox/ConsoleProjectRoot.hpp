#pragma once

#include "ConsoleCommands.hpp"

#include <filesystem>
#include <optional>

namespace ForgeSim::Sandbox
{
    // A setting for subsequent opens, separate from the loaded scene's root.
    class ConsoleProjectRoot final
    {
    public:
        explicit ConsoleProjectRoot(std::filesystem::path initialRoot,
            std::optional<std::filesystem::path> overrideRoot = std::nullopt)
            : m_Fallback(initialRoot), m_Path(overrideRoot ? std::filesystem::canonical(*overrideRoot) : std::move(initialRoot)),
              m_Override(overrideRoot ? std::optional{m_Path} : std::nullopt) {}

        [[nodiscard]] const std::filesystem::path& Path() const noexcept { return m_Path; }

        [[nodiscard]] const std::optional<std::filesystem::path>& Override() const noexcept
        { return m_Override; }

        void UseAutomaticDiscovery()
        {
            m_Path = m_Fallback;
            m_Override.reset();
        }

        [[nodiscard]] ConsoleCommandResult Execute(std::span<const std::string> arguments)
        {
            if (arguments.size() > 1) return {"Usage: project.root [directory]", true};
            if (!arguments.empty())
            {
                if (arguments[0].empty()) return {"Project root must name a directory.", true};
                const std::u8string utf8(arguments[0].begin(), arguments[0].end());
                const auto candidate = std::filesystem::canonical(
                    std::filesystem::absolute(std::filesystem::path(utf8)));
                if (!std::filesystem::is_directory(candidate))
                    return {"Project root must name an existing directory.", true};
                m_Path = candidate;
                m_Override = candidate;
            }
            const auto utf8 = m_Path.generic_u8string();
            return {"Project root for scene opens: " + std::string(utf8.begin(), utf8.end()) +
                (m_Override ? " (explicit override)" : " (legacy fallback; project manifests take precedence)") +
                (arguments.empty() ? "" : "\nOpen or reopen a scene to apply this root.")};
        }

    private:
        std::filesystem::path m_Fallback;
        std::filesystem::path m_Path;
        std::optional<std::filesystem::path> m_Override;
    };
}
