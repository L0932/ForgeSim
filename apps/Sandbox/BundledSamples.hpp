#pragma once

#include <array>
#include <string_view>

namespace ForgeSim::Sandbox
{
    struct BundledSample
    {
        const char* label;
        const char* filename;
        const char* description;
        const char* preview;
    };

    // Filenames are relative to the bundled assets/scenes directory.
    inline constexpr std::array BundledSamples{
        BundledSample{"Starter: Directional Shadows", "DirectionalShadowTest.forgesim.json",
            "Explore object transforms, visibility, and directional shadows.", "DirectionalShadows.png"},
        BundledSample{"Material Gallery", "MaterialGallery.scene.json",
            "Compare metallic and nonmetallic materials across roughness values.", "MaterialGallery.png"}
    };

    [[nodiscard]] inline bool IsBundledSample(std::string_view filename)
    {
        for (const auto& sample : BundledSamples)
            if (filename == sample.filename)
                return true;
        return false;
    }
}
