#pragma once

#include "ConsoleCommands.hpp"

namespace ForgeSim::Sandbox
{
    inline ConsoleCommandResult ExecuteTextureFallbackCommand(
        std::span<const std::string> arguments, bool& enabled)
    {
        if (arguments.size() > 1 || (!arguments.empty() &&
            arguments[0] != "on" && arguments[0] != "off"))
            return {"Usage: textures.fallback [on|off]", true};

        if (!arguments.empty()) enabled = arguments[0] == "on";
        return {std::string("Texture fallbacks: ") + (enabled ? "on" : "off") +
            ". Applies to subsequent model loads."};
    }
}
