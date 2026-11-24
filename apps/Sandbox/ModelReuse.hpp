#pragma once

#include <array>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

namespace ForgeSim::Sandbox
{
    struct ModelReuseStamp
    {
        std::uintmax_t bytes = 0;
        std::filesystem::file_time_type modified;
        bool operator==(const ModelReuseStamp&) const = default;
    };

    // Only self-contained GLBs qualify. Other imports keep the full reload path,
    // so changes to external buffers/images cannot leave cached GPU data stale.
    // Metadata checks intentionally do not detect edits preserving both size/time.
    inline std::optional<ModelReuseStamp> ReadModelReuseStamp(const std::filesystem::path& path)
    {
        try
        {
            const ModelReuseStamp before{std::filesystem::file_size(path), std::filesystem::last_write_time(path)};
            std::ifstream file(path, std::ios::binary);
            std::array<unsigned char, 20> header{};
            if (!file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size()))) return std::nullopt;
            const auto u32 = [&](std::size_t at)
            {
                return std::uint32_t(header[at]) | (std::uint32_t(header[at+1]) << 8) |
                    (std::uint32_t(header[at+2]) << 16) | (std::uint32_t(header[at+3]) << 24);
            };
            const auto jsonBytes = u32(12);
            if (u32(0) != 0x46546c67 || u32(4) != 2 || u32(8) != before.bytes ||
                u32(16) != 0x4e4f534a || jsonBytes > 16 * 1024 * 1024 ||
                jsonBytes > before.bytes - header.size()) return std::nullopt;
            std::string source(jsonBytes, '\0');
            if (!file.read(source.data(), static_cast<std::streamsize>(source.size()))) return std::nullopt;
            const auto document = nlohmann::json::parse(source, nullptr, false);
            if (!document.is_object()) return std::nullopt;
            const auto hasExternalUri = [](const auto& self, const nlohmann::json& value) -> bool
            {
                if (value.is_object())
                    for (auto it = value.begin(); it != value.end(); ++it)
                    {
                        if (it.key() == "uri" && (!it.value().is_string() ||
                            !it.value().get_ref<const std::string&>().starts_with("data:"))) return true;
                        if (self(self, it.value())) return true;
                    }
                if (value.is_array())
                    for (const auto& child : value) if (self(self, child)) return true;
                return false;
            };
            if (hasExternalUri(hasExternalUri, document)) return std::nullopt;
            const ModelReuseStamp after{std::filesystem::file_size(path), std::filesystem::last_write_time(path)};
            if (before != after) return std::nullopt;
            return after;
        }
        catch (const std::exception&)
        {
            // Reuse is optional. Import remains responsible for reporting errors.
            return std::nullopt;
        }
    }
}
