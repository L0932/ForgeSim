#include "ModelImportLocation.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    struct Temp
    {
        fs::path path = fs::temp_directory_path() / ("ForgeSimImport-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Temp() { fs::create_directories(path); }
        ~Temp() { std::error_code ignored; fs::remove_all(path, ignored); }
    };
}
int main()
{
    try
    {
        Temp temp;
        const auto project = temp.path / "asset project";
        const auto models = project / fs::path(u8"Hidden Alley \u00e9");
        fs::create_directories(models);
        const auto model = models / "alley.glb";
        std::ofstream(model) << "placeholder";
        auto result = ResolveModelImportLocation(model);
        Require(result.root == fs::canonical(models) && result.model.filename() == "alley.glb",
            "Standalone model uses its containing folder");
        std::ofstream(project / "ForgeSim.project.json") << R"({"version":1,"assetRoot":"."})";
        result = ResolveModelImportLocation(model);
        Require(result.root == fs::canonical(project) && fs::equivalent(result.model, model),
            "Raw model discovers enclosing manifest");
        result = ResolveModelImportLocation(model, models);
        Require(result.root == fs::canonical(models), "Explicit containing root wins");
        bool failed = false;
        try { static_cast<void>(ResolveModelImportLocation(model, temp.path / "missing")); }
        catch (const std::exception&) { failed = true; }
        Require(failed, "Missing override rejected before loading");
        fs::create_directories(temp.path / "unrelated");
        failed = false;
        try { static_cast<void>(ResolveModelImportLocation(model, temp.path / "unrelated")); }
        catch (const std::invalid_argument&) { failed = true; }
        Require(failed, "Unrelated explicit root cannot generate dot-dot references");
        std::cout << "Direct model import root discovery and preflight passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
