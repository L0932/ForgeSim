#include "SceneAssetRoot.hpp"
#include "ConsoleProjectRoot.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    struct TemporaryDirectory
    {
        fs::path path = fs::temp_directory_path() / ("ForgeSimRoot-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::path original = fs::current_path();
        TemporaryDirectory() { fs::create_directories(path); }
        ~TemporaryDirectory()
        {
            std::error_code ignored;
            fs::current_path(original, ignored);
            fs::remove_all(path, ignored);
        }
    };
}

int main()
{
    try
    {
        TemporaryDirectory temp;
        auto pack = temp.path / fs::path(u8"Sample pack \u00e9");
        // Exercise canonicalization through an alternate spelling on systems
        // that permit directory symlinks. Windows CI also exercises temp aliases.
        fs::create_directories(temp.path / "physical");
        std::error_code aliasError;
        fs::create_directory_symlink(temp.path / "physical", temp.path / "alias", aliasError);
        if (!aliasError) pack = temp.path / "alias" / fs::path(u8"Sample pack \u00e9");
        const auto fallback = temp.path / "application";
        const auto overrideRoot = temp.path / "override";
        fs::create_directories(pack / "scenes/nested");
        fs::create_directories(pack / "content");
        fs::create_directories(fallback);
        fs::create_directories(overrideRoot);
        auto scene = pack / "scenes/nested/example.scene.json";
        std::ofstream(scene) << "{}";
        auto manifest = pack / "ForgeSim.project.json";
        const auto resolve = [&] { return ResolveSceneAssetRoot(scene, std::nullopt, fallback); };
        const auto rejects = [&](std::string_view content)
        {
            std::ofstream(manifest) << content;
            try { static_cast<void>(resolve()); }
            catch (const std::exception&) { return true; }
            return false;
        };
        auto result = resolve();
        Require(result.root == fs::canonical(fallback) &&
            result.source == SceneAssetRootSource::LegacyFallback && !result.manifest,
            "Scenes without a manifest retain the caller's legacy root");
        std::ofstream(manifest) << R"({"version":1,"assetRoot":"content"})";
        fs::current_path(fallback);
        result = resolve();
        Require(result.source == SceneAssetRootSource::ProjectManifest,
            "Ancestor discovery must select the project manifest independently of cwd");
        Require(fs::equivalent(result.root, pack / "content"),
            "Ancestor manifest must resolve assetRoot relative to its own directory");
        // Discovery canonicalizes the scene path. On Windows this can expand
        // a temporary directory's short name or change its case, so the returned
        // manifest may identify the same file with a different path spelling.
        Require(result.manifest.has_value(), "Ancestor discovery must return its manifest path");
        Require(fs::equivalent(*result.manifest, manifest),
            "Ancestor discovery must return the expected manifest file");
        ConsoleProjectRoot setting{fallback};
        Require(!setting.Override(), "Default root is not an explicit override");
        static_cast<void>(setting.Execute({}));
        Require(!setting.Override(), "Querying root does not disable discovery");
        Require(ResolveSceneAssetRoot(scene, setting.Override(), setting.Path()).root == result.root,
            "Editor setting allows manifest discovery by default");
        const auto bytes = overrideRoot.generic_u8string();
        const std::vector<std::string> args{std::string(bytes.begin(), bytes.end())};
        Require(!setting.Execute(args).error && setting.Override().has_value(),
            "Console set creates an explicit override");
        Require(ResolveSceneAssetRoot(scene, setting.Override(), fallback).root == fs::canonical(overrideRoot),
            "Console override wins over project manifest");
        setting.UseAutomaticDiscovery();
        Require(!setting.Override() && setting.Path() == fallback,
            "Automatic mode clears the override and restores fallback");
        ConsoleProjectRoot launched{fallback, overrideRoot};
        Require(launched.Override() == fs::canonical(overrideRoot), "CLI root remains an explicit editor override");
        const auto moved = temp.path / "relocated pack";
        fs::rename(pack, moved);
        pack = moved;
        scene = pack / "scenes/nested/example.scene.json";
        manifest = pack / "ForgeSim.project.json";
        Require(resolve().root == fs::canonical(pack / "content"), "Complete pack can relocate");
        const auto nested = pack / "scenes/ForgeSim.project.json";
        std::ofstream(nested) << R"({"version":1,"assetRoot":"."})";
        Require(resolve().root == fs::canonical(pack / "scenes"), "Nearest manifest wins");
        std::ofstream(nested) << "invalid";
        bool failed = false;
        try { static_cast<void>(resolve()); } catch (const std::exception&) { failed = true; }
        Require(failed, "Invalid nearest manifest must not fall through to a parent");
        result = ResolveSceneAssetRoot(scene, overrideRoot, fallback);
        Require(result.root == fs::canonical(overrideRoot) && !result.manifest &&
            result.source == SceneAssetRootSource::ExplicitOverride,
            "Explicit override bypasses manifest discovery");
        fs::remove(nested);
        for (const auto* invalid : {"[]", "{}", R"({"version":2,"assetRoot":"."})",
            R"({"version":1.0,"assetRoot":"."})", R"({"version":1,"assetRoot":42})",
            R"({"version":1,"assetRoot":""})", R"({"version":1,"assetRoot":"../outside"})",
            R"({"version":1,"assetRoot":"/absolute"})", R"({"version":1,"assetRoot":"C:/assets"})",
            R"({"version":1,"assetRoot":"content\\nested"})",
            R"({"version":1,"assetRoot":"missing"})",
            R"({"version":1,"assetRoot":"scenes/nested/example.scene.json"})"})
            Require(rejects(invalid), "Invalid manifest rejected");
        std::ofstream(manifest) << R"({"version":1,"assetRoot":"."})";
        Require(resolve().root == fs::canonical(pack), "Dot root uses manifest directory");
        failed = false;
        try { static_cast<void>(ResolveSceneAssetRoot(scene, pack / "missing", fallback)); }
        catch (const std::exception&) { failed = true; }
        Require(failed, "Invalid override must not silently fall back");
        std::cout << "Scene asset root discovery, precedence, relocation, and validation passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
