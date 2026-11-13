#pragma once
#include "ScreenshotImage.hpp"

namespace ForgeSim::Sandbox
{
    [[nodiscard]] ScreenshotImage CaptureSceneTexture(unsigned int texture);
    [[nodiscard]] ScreenshotImage CaptureEditorBackbuffer(std::uint32_t width,std::uint32_t height);
}
