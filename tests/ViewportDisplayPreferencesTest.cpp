#include "ViewportDisplayPreferences.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

using ForgeSim::Sandbox::ViewportDisplayPreferences;

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    struct TemporaryDirectory
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() /
            ("ForgeSim-viewport-preferences-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        ~TemporaryDirectory()
        {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    };
}

int main()
{
    try
    {
        TemporaryDirectory directory;
        const auto path = directory.path / "Settings" / "viewport-display.json";
        ViewportDisplayPreferences preferences{false, false, true};
        preferences.Load(path);
        Require(preferences.showReferenceGrid && preferences.showSceneLights &&
            !preferences.showObjectBounds, "Missing preferences must restore defaults");
        // Exercise every combination, including false values rather than only default-on fields.
        for (unsigned mask = 0; mask != 8; ++mask)
        {
            const ViewportDisplayPreferences expected{
                (mask & 1) != 0, (mask & 2) != 0, (mask & 4) != 0};
            Require(expected.Save(path), "Save must create the settings directory");
            ViewportDisplayPreferences reopened;
            reopened.Load(path);
            Require(reopened.showReferenceGrid == expected.showReferenceGrid &&
                reopened.showSceneLights == expected.showSceneLights &&
                reopened.showObjectBounds == expected.showObjectBounds,
                "Preferences must survive reopening");
        }
        const auto write = [&](const char* value)
        {
            std::ofstream output(path);
            output << value;
        };
        write(R"({"version":1,"showReferenceGrid":false,"showSceneLights":"false","showObjectBounds":1})");
        preferences.Load(path);
        Require(!preferences.showReferenceGrid && preferences.showSceneLights &&
            !preferences.showObjectBounds, "Invalid fields use defaults independently");
        for (const auto* invalid : {"{", "[]", "null", "{}",
            R"({"version":2,"showReferenceGrid":false})",
            R"({"version":"1","showReferenceGrid":false})"})
        {
            write(invalid);
            preferences.Load(path);
            Require(preferences.showReferenceGrid && preferences.showSceneLights &&
                !preferences.showObjectBounds, "Invalid or unsupported file uses defaults");
        }
        Require(!preferences.Save(path / "child.json"), "Unwritable path must report failure");
        Require(preferences.showReferenceGrid && preferences.showSceneLights &&
            !preferences.showObjectBounds, "Save failure must preserve current preferences");
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
