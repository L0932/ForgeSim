#include "ModelReuse.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

using namespace ForgeSim::Sandbox;
namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    void WriteGlb(const std::filesystem::path& path, std::string json)
    {
        while (json.size() % 4) json += ' ';
        std::ofstream out(path, std::ios::binary);
        const auto word = [&](std::uint32_t value)
        {
            for (unsigned shift = 0; shift < 32; shift += 8) out.put(static_cast<char>((value >> shift) & 255));
        };
        word(0x46546c67); word(2); word(static_cast<std::uint32_t>(20 + json.size()));
        word(static_cast<std::uint32_t>(json.size())); word(0x4e4f534a); out << json;
    }
    struct TempDirectory
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() /
            ("ForgeSimModelReuse-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        TempDirectory() { std::filesystem::create_directory(path); }
        ~TempDirectory() { std::error_code error; std::filesystem::remove_all(path, error); }
    };
}
int main()
{
    try
    {
        TempDirectory temp;
        const auto path = temp.path / "model.glb";
        Require(!ReadModelReuseStamp(path), "Missing source cannot be reused");
        WriteGlb(path, R"({"asset":{"version":"2.0"},"buffers":[{"byteLength":0}]})");
        const auto first = ReadModelReuseStamp(path);
        Require(first.has_value(), "Embedded GLB must qualify");
        Require(ReadModelReuseStamp(path) == first, "Unchanged source must match");
        std::filesystem::last_write_time(path, first->modified + std::chrono::seconds(2));
        Require(ReadModelReuseStamp(path) != first, "Timestamp change must invalidate");
        WriteGlb(path, R"({"asset":{"version":"2.0"},"extras":{"changed":true,"longer_payload":"different size"}})");
        std::filesystem::last_write_time(path, first->modified);
        Require(ReadModelReuseStamp(path) != first, "Size change must invalidate even with restored time");
        WriteGlb(path, R"({"buffers":[{"uri":"geometry.bin"}]})");
        Require(!ReadModelReuseStamp(path), "External buffer must bypass reuse");
        WriteGlb(path, R"({"images":[{"uri":"textures/image%20one.png"}]})");
        Require(!ReadModelReuseStamp(path), "External image must bypass reuse");
        WriteGlb(path, R"({"extensions":{"example":{"uri":"https://example.com/asset"}}})");
        Require(!ReadModelReuseStamp(path), "Extension URI must bypass reuse");
        WriteGlb(path, R"({"images":[{"uri":"data:image/png;base64,AAAA"}]})");
        Require(ReadModelReuseStamp(path).has_value(), "Inline data has no external dependency");
        WriteGlb(path, "{broken}");
        Require(!ReadModelReuseStamp(path), "Corrupt JSON must bypass reuse");
        WriteGlb(path, R"({"images":[{"uri":123}]})");
        Require(!ReadModelReuseStamp(path), "Malformed URI must bypass reuse");
        WriteGlb(path, "{}");
        std::filesystem::resize_file(path, 12);
        Require(!ReadModelReuseStamp(path), "Truncated GLB must bypass reuse");
        { std::ofstream out(path); out << R"({"asset":{"version":"2.0"}})"; }
        Require(!ReadModelReuseStamp(path), "JSON glTF must use normal import");
        std::cout << "Model reuse dependency and invalidation checks passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n'; return 1;
    }
}
