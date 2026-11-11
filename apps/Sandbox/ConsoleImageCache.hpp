#pragma once

#include "ConsoleCommands.hpp"

namespace ForgeSim::Sandbox
{
    inline ConsoleCommandResult ExecuteImageCacheCommand(
        std::span<const std::string> arguments, bool& enabled)
    {
        if (arguments.size() > 1 || (!arguments.empty() &&
            arguments[0] != "on" && arguments[0] != "off"))
            return {"Usage: images.cache [on|off]", true};

        if (!arguments.empty()) enabled = arguments[0] == "on";
        return {std::string("Decoded-image caching: ") + (enabled ? "on" : "off") +
            ". Applies to subsequent model loads."};
    }
}
