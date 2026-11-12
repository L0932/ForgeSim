#pragma once

#include <array>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct BundledSample
    {
        const char* label;
        const char* filename;
    };

    // Filenames are relative to the bundled assets/scenes directory.
    inline constexpr std::array BundledSamples{
        BundledSample{"Starter: Directional Shadows", "DirectionalShadowTest.forgesim.json"},
        BundledSample{"Material Gallery", "MaterialGallery.scene.json"}
    };

    [[nodiscard]] inline bool IsBundledSample(std::string_view filename)
    {
        for (const auto& sample : BundledSamples)
            if (filename == sample.filename)
                return true;
        return false;
    }
}
