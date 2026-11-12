#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <ForgeSim/Assets/ImageLoader.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>

namespace ForgeSim::Sandbox
{
    struct SamplePreview
    {
        std::optional<Renderer::OpenGL::Texture2D> texture;
        float aspect = 2.56f;
        bool attempted = false;

        // Called lazily from the browser, while the application GL context is alive.
        // Failure affects only the preview; the scene remains available to open.
        void Load(const std::filesystem::path& path)
        {
            if (attempted)
                return;
            attempted = true;
            try
            {
                const auto image = Assets::LoadImage(path);
                texture.emplace(image.width, image.height, image.pixels,
                    Renderer::OpenGL::TextureColorSpace::Linear);
                aspect = static_cast<float>(image.width) / static_cast<float>(image.height);
            }
            catch (const std::exception& error)
            {
                Core::LogWarning(std::string{"Sample preview unavailable: "} + error.what());
            }
        }
    };
}
