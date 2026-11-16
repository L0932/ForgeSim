#pragma once

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace ForgeSim::Sandbox
{
    // Editor preferences deliberately live outside scene documents and undo history.
    struct ViewportDisplayPreferences
    {
        static constexpr int SchemaVersion = 1;

        bool showReferenceGrid = true;
        bool showSceneLights = true;
        bool showObjectBounds = false;

        void Load(const std::filesystem::path& path) noexcept
        {
            *this = {};
            try
            {
                std::ifstream input(path);
                const auto json = nlohmann::json::parse(input, nullptr, false);
                if (!json.is_object() || !json.contains("version") ||
                    !json["version"].is_number_integer() || json["version"] != SchemaVersion) return;
                const auto read = [&](const char* key, bool& destination)
                {
                    const auto item = json.find(key);
                    if (item != json.end() && item->is_boolean()) destination = item->get<bool>();
                };
                read("showReferenceGrid", showReferenceGrid);
                read("showSceneLights", showSceneLights);
                read("showObjectBounds", showObjectBounds);
            }
            catch (...) { /* Missing or invalid preferences cannot prevent startup. */ }
        }

        [[nodiscard]] bool Save(const std::filesystem::path& path) const noexcept
        {
            try
            {
                if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
                std::ofstream output(path, std::ios::trunc);
                if (!output) return false;
                output << nlohmann::json{{"version", SchemaVersion},
                    {"showReferenceGrid", showReferenceGrid},
                    {"showSceneLights", showSceneLights},
                    {"showObjectBounds", showObjectBounds}}.dump(2) << '\n';
                output.close();
                return !output.fail();
            }
            catch (...) { return false; }
        }
    };
}
