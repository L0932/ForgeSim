#include "EnvironmentImageImport.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using ForgeSim::Sandbox::ImportEnvironmentImage;

void Require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}

struct Fixture
{
    fs::path path = fs::temp_directory_path() /
        ("ForgeSim-environment-import-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture() { fs::create_directories(path / "assets"); fs::create_directory(path / "external"); }
    ~Fixture() { std::error_code ignored; fs::remove_all(path, ignored); }
};

int main()
{
    try
    {
        Fixture fixture;
        const auto root = fixture.path / "assets";
        const auto source = fixture.path / "external" / "Studio.hdr";
        std::ofstream(source) << "HDR source bytes";
        std::string reference;
        ImportEnvironmentImage(source, root, [&](const std::string& asset) { reference = asset; });
        Require(reference == "environments/Studio.hdr" && fs::file_size(root / reference) == fs::file_size(source),
            "External HDR must be copied and referenced relative to the asset root");
        const auto firstReference = reference;
        std::ofstream(root / firstReference) << "Preserve existing asset";
        ImportEnvironmentImage(source, root, [&](const std::string& asset) { reference = asset; });
        Require(reference == "environments/Studio_1.hdr", "Existing image must not be overwritten");
        std::string existingContents;
        std::getline(std::ifstream{root / firstReference}, existingContents);
        Require(existingContents == "Preserve existing asset", "Import must retain existing asset contents");
        ImportEnvironmentImage(root / firstReference, root, [&](const std::string& asset) { reference = asset; });
        Require(reference == firstReference && !fs::exists(root / "environments/Studio_2.hdr"),
            "In-root image must be reused without copying");
        bool failed = false;
        try
        {
            ImportEnvironmentImage(source, root, [&](const std::string& asset)
            {
                Require(fs::exists(root / asset), "Load callback must receive an existing copied image");
                throw std::runtime_error("HDR decoding failed");
            });
        }
        catch (const std::runtime_error&) { failed = true; }
        Require(failed && !fs::exists(root / "environments/Studio_2.hdr") && fs::exists(root / firstReference),
            "Failed loading must remove only the new copy");
        try
        {
            ImportEnvironmentImage(root / firstReference, root,
                [&](const std::string&) { throw std::runtime_error("Existing image load failed"); });
        }
        catch (const std::runtime_error&) {}
        Require(fs::exists(root / firstReference), "Failed reuse must never remove an existing project image");
        const auto unicodeName = fs::path{u8"marbre_é.hdr"};
        std::ofstream(fixture.path / "external" / unicodeName) << "Unicode HDR";
        ImportEnvironmentImage(fixture.path / "external" / unicodeName, root,
            [&](const std::string& asset) { reference = asset; });
        const auto expected = (fs::path{"environments"} / unicodeName).generic_u8string();
        Require(reference == std::string(expected.begin(), expected.end()), "Asset references must preserve UTF-8");
        bool called = false;
        try { ImportEnvironmentImage(fixture.path / "missing.hdr", root, [&](const std::string&) { called = true; }); }
        catch (const fs::filesystem_error&) {}
        Require(!called, "Missing image must not reach environment replacement");
        std::cout << "Environment image references, external copy, collisions, rollback and UTF-8 passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
