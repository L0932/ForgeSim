#include "ExternalSample.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    std::string Utf8(const fs::path& path)
    {
        const auto bytes = path.generic_u8string();
        return {bytes.begin(), bytes.end()};
    }
    struct TemporaryDirectory
    {
        fs::path path = fs::temp_directory_path() / ("ForgeSimExternalSample-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        TemporaryDirectory() { fs::create_directories(path); }
        ~TemporaryDirectory() { std::error_code error; fs::remove_all(path, error); }
    };
}
int main()
{
    try
    {
        TemporaryDirectory temp;
        // UTF-8 paths and spaces must survive the ImGui -> filesystem boundary.
        const auto root = temp.path / fs::path(u8"Sample assets \u00e9");
        fs::create_directories(root / "scenes");
        const auto scene = root / "scenes/cathedral.scene.json";
        std::ofstream(scene) << "{}";
        const auto result = ResolveExternalSample("scenes/cathedral.scene.json", Utf8(root));
        Require(result.scene == fs::canonical(scene) && result.root == fs::canonical(root),
            "Relative scene path resolves against chosen root, independently of cwd");
        Require(ResolveExternalSample(Utf8(scene), Utf8(root)).scene == result.scene,
            "Absolute scene paths remain supported");
        const auto rejects = [](std::string_view sceneText, std::string_view rootText)
        {
            try { static_cast<void>(ResolveExternalSample(sceneText, rootText)); }
            catch (const std::exception&) { return true; }
            return false;
        };
        Require(rejects("", Utf8(root)), "Missing scene rejected");
        Require(rejects(Utf8(scene), "relative root"), "Ambiguous relative root rejected");
        Require(rejects("missing.json", Utf8(root)), "Missing scene rejected");
        Require(rejects(Utf8(scene), Utf8(scene)), "File cannot be a project root");
        Require(rejects("scenes", Utf8(root)), "Directory cannot be a scene");
        std::ofstream(root / "cathedral.glb") << "test";
        Require(rejects("cathedral.glb", Utf8(root)), "Model file needs a scene document");
        std::cout << "External sample root resolution and UTF-8 paths passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
