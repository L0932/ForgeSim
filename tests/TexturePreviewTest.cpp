#include "TexturePreview.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

using namespace ForgeSim;
namespace
{
    void Require(bool condition, const char* message)
    { if (!condition) throw std::runtime_error(message); }
}
int main()
{
    try
    {
        const Assets::ImageData source{2, 1, 4, {128, 64, 192, 0, 12, 240, 30, 127}};
        const auto original = source.pixels;
        const auto rgb = Sandbox::CreateTexturePreview(source, Sandbox::TexturePreviewMode::Rgb);
        Require(rgb.pixels == std::vector<std::uint8_t>{128,64,192,255,12,240,30,255},
            "RGB must preserve source bytes without gamma changes or alpha hiding content");
        const auto roughness = Sandbox::CreateTexturePreview(source, Sandbox::TexturePreviewMode::Roughness);
        Require(roughness.pixels == std::vector<std::uint8_t>{64,64,64,255,240,240,240,255},
            "Roughness must extract G as grayscale without applying material factors");
        const auto metallic = Sandbox::CreateTexturePreview(source, Sandbox::TexturePreviewMode::Metallic);
        Require(metallic.pixels == std::vector<std::uint8_t>{192,192,192,255,30,30,30,255},
            "Metallic must extract B as grayscale");
        Require(source.pixels == original, "Preview construction must not modify source pixels");
        for (const auto& dimensions : {std::pair{2048u,1024u}, std::pair{1u,2048u}, std::pair{1024u,2048u}})
        {
            Assets::ImageData large{dimensions.first, dimensions.second, 4, {}};
            large.pixels.resize(static_cast<std::size_t>(large.width) * large.height * 4, 97);
            const auto preview = Sandbox::CreateTexturePreview(large, Sandbox::TexturePreviewMode::Rgb);
            Require(preview.IsValid() && std::max(preview.width, preview.height) == 512 &&
                preview.width >= 1 && preview.height >= 1, "Preview extent must be bounded, including thin images");
            Require(preview.pixels[0] == 97 && preview.pixels.back() == 255,
                "Resizing must retain constant RGB and opaque preview alpha");
        }
        // A one-pixel feature must survive full inspection even when a thumbnail
        // cannot represent every source texel.
        Assets::ImageData detailed{1024, 1, 4, {}};
        detailed.pixels.resize(1024 * 4, 0);
        detailed.pixels[411 * 4 + 1] = 123;
        detailed.pixels[411 * 4 + 2] = 231;
        const auto full = Sandbox::CreateTexturePreview(detailed, Sandbox::TexturePreviewMode::Metallic,
            Sandbox::TexturePreviewResolution::Full);
        Require(full.width == 1024 && full.height == 1 && full.pixels[411 * 4] == 231 &&
            full.pixels[410 * 4] == 0 && full.pixels[412 * 4] == 0,
            "Full inspection must preserve isolated source detail and exact pixel positions");
        const auto fullRgb = Sandbox::CreateTexturePreview(detailed, Sandbox::TexturePreviewMode::Rgb,
            Sandbox::TexturePreviewResolution::Full);
        Require(fullRgb.pixels[411 * 4 + 1] == 123 && fullRgb.pixels[411 * 4 + 2] == 231,
            "Full RGB inspection must not resample source channels");
        auto broken = source;
        broken.pixels.pop_back();
        bool rejected = false;
        try { static_cast<void>(Sandbox::CreateTexturePreview(broken, Sandbox::TexturePreviewMode::Rgb)); }
        catch (const std::invalid_argument&) { rejected = true; }
        Require(rejected, "Truncated source pixels must not be read");
        std::cout << "Texture preview source values, channels, alpha and sizing passed\n";
    }
    catch (const std::exception& error)
    { std::cerr << error.what() << '\n'; return 1; }
}
