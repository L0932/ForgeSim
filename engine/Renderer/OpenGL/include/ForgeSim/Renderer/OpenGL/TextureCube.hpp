#pragma once

#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
	class TextureCube final
	{
	public:
		TextureCube(
			std::uint32_t size,
			std::uint32_t mipLevelCount);

		~TextureCube();

		TextureCube(
			const TextureCube&) = delete;

		TextureCube& operator=(
			const TextureCube&) = delete;

		TextureCube(
			TextureCube&& other) noexcept;

		TextureCube& operator=(
			TextureCube&& other) noexcept;

		void Bind(
			std::uint32_t textureUnit) const noexcept;

		void AttachFaceToBoundFramebuffer(
			std::uint32_t faceIndex,
			std::uint32_t mipLevel) const;

		void GenerateMipmaps() const noexcept;

		[[nodiscard]] std::uint32_t Size()
			const noexcept;

		[[nodiscard]] std::uint32_t MipLevelCount()
			const noexcept;

	private:
		unsigned int m_Handle = 0;
		std::uint32_t m_Size = 0;
		std::uint32_t m_MipLevelCount = 0;
	};
}