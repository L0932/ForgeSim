#include "ExternalSample.hpp"
#include "SceneAssetRoot.hpp"

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
        std::ofstream(root / "ForgeSim.project.json") << R"({"version":1,"assetRoot":"."})";
        const auto selected = ResolveExternalSample(Utf8(scene));
        Require(selected == fs::canonical(scene), "Browser selection preserves UTF-8 and spaces");
        const auto fallback = temp.path / "application";
        fs::create_directories(fallback);
        Require(ResolveSceneAssetRoot(selected, std::nullopt, fallback).root == fs::canonical(root),
            "Browser scene discovers its pack root without a root field");
        const auto moved = temp.path / "relocated pack";
        fs::rename(root, moved);
        const auto movedScene = ResolveExternalSample(Utf8(moved / "scenes/cathedral.scene.json"));
        Require(ResolveSceneAssetRoot(movedScene, std::nullopt, fallback).root == fs::canonical(moved),
            "Browser scene continues to resolve after moving the pack");
        const auto rejects = [](std::string_view sceneText)
        {
            try { static_cast<void>(ResolveExternalSample(sceneText)); }
            catch (const std::exception&) { return true; }
            return false;
        };
        Require(rejects(""), "Missing scene rejected");
        Require(rejects("scenes/cathedral.scene.json"), "Relative scene requires an absolute selection");
        Require(rejects(Utf8(moved / "missing.json")), "Missing scene rejected");
        Require(rejects(Utf8(moved / "scenes")), "Directory cannot be a scene");
        std::ofstream(moved / "cathedral.glb") << "test";
        Require(rejects(Utf8(moved / "cathedral.glb")), "Model file needs a scene document");
        std::cout << "External sample root resolution and UTF-8 paths passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
