#pragma once

#include <string_view>

namespace ForgeSim::Sandbox
{
    inline constexpr std::string_view BuiltinCubeReference = "@builtin/cube";

        [[nodiscard]] inline bool IsValidSceneAssetPath(std::string_view path)
        {
            if (path.empty())
            {
                return false;
            }

            // Reject platform-specific separators, drive prefixes,
            // embedded nulls, and other control characters.
            for (const char character : path)
            {
                const auto byte =
                    static_cast<unsigned char>(character);

                if (character == '\\' ||
                    character == ':' ||
                    byte < 32 ||
                    byte == 127)
                {
                    return false;
                }
            }

            // Require nonempty, ordinary relative components.
            // This also rejects leading/trailing or repeated slashes.
            std::size_t begin = 0;

            while (begin < path.size())
            {
                const std::size_t end = path.find('/', begin);

                const std::string_view component =
                    path.substr(
                        begin,
                        end == std::string_view::npos
                        ? path.size() - begin
                        : end - begin);

                if (component.empty() ||
                    component == "." ||
                    component == "..")
                {
                    return false;
                }

                if (end == std::string_view::npos)
                {
                    return true;
                }

                begin = end + 1;
            }

            return false;
        }

}
