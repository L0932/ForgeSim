#pragma once
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace ForgeSim::Sandbox
{
    struct ScreenshotImage
    {
        std::uint32_t width=0, height=0;
        std::vector<std::uint8_t> rgba;
    };
    [[nodiscard]] std::size_t ScreenshotByteCount(std::uint32_t width,std::uint32_t height);
    // OpenGL readback is bottom-up. PNG and ordinary image viewers are top-down.
    void NormalizeScreenshot(ScreenshotImage& image);
    void WriteScreenshotPng(const std::filesystem::path& path,const ScreenshotImage& image);
}
