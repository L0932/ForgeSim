#pragma once

#include "TexturePreview.hpp"

#include <ForgeSim/Assets/SceneAsset.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>
#include <glad/gl.h>

#include <map>
#include <optional>
#include <tuple>
#include <string>

namespace ForgeSim::Sandbox
{
    // Owned by the renderer and used only on the GL/UI thread.
    class TexturePreviewCache
    {
    public:
        void Invalidate() noexcept { ++m_Generation; }
        [[nodiscard]] std::uint64_t Generation() const noexcept { return m_Generation; }

        void BeginFrame()
        {
            // UI draw commands may still reference previews when a scene is
            // replaced. Release them only after that frame has been submitted.
            std::erase_if(m_Textures, [this](const auto& entry)
                { return std::get<0>(entry.first) != m_Generation ||
                    (std::get<4>(entry.first) == TexturePreviewResolution::Full && entry.first != m_LastFull); });
            m_LastFull.reset();
        }

        [[nodiscard]] std::uint32_t Get(const Assets::SceneAsset& asset,
            std::size_t imageIndex, TexturePreviewMode mode,
            TexturePreviewResolution resolution = TexturePreviewResolution::Thumbnail)
        {
            if (imageIndex >= asset.textureResources.images.size()) return 0;
            const Key key{m_Generation, reinterpret_cast<std::uintptr_t>(&asset), imageIndex, mode, resolution};
            if (resolution == TexturePreviewResolution::Full) m_LastFull = key;
            const auto [entry, inserted] = m_Textures.try_emplace(key);
            if (inserted)
            {
                try
                {
                    const auto image = CreateTexturePreview(asset.textureResources.images[imageIndex], mode, resolution);
                    // RGBA8 prevents a second sRGB decode in ImGui's shader.
                    entry->second.emplace(image.width, image.height, image.pixels,
                        Renderer::OpenGL::TextureColorSpace::Linear);
                    const auto handle = entry->second->Handle();
                    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, resolution == TexturePreviewResolution::Full
                        ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
                    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, resolution == TexturePreviewResolution::Full
                        ? GL_NEAREST : GL_LINEAR);
                    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                catch (const std::exception& error)
                {
                    // Cache failure too; do not retry/log on every UI frame.
                    Core::LogWarning(std::string{"Texture preview unavailable: "} + error.what());
                }
            }
            return entry->second ? entry->second->Handle() : 0;
        }

    private:
        using Key = std::tuple<std::uint64_t, std::uintptr_t, std::size_t, TexturePreviewMode, TexturePreviewResolution>;
        std::optional<Key> m_LastFull;
        std::uint64_t m_Generation = 0;
        std::map<Key, std::optional<Renderer::OpenGL::Texture2D>> m_Textures;
    };
}
