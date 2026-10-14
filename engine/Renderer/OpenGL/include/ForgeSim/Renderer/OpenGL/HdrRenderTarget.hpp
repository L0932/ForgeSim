#pragma once

#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
	class HdrRenderTarget final
	{
	public:
		HdrRenderTarget(
			std::uint32_t width,
			std::uint32_t height);

		~HdrRenderTarget();

		HdrRenderTarget(
			const HdrRenderTarget&) = delete;

		HdrRenderTarget& operator=(
			const HdrRenderTarget&) = delete;

		HdrRenderTarget(
			HdrRenderTarget&&) = delete;

		HdrRenderTarget& operator=(
			HdrRenderTarget&&) = delete;

		void Resize(
			std::uint32_t width,
			std::uint32_t height);

		void Bind() const noexcept;

		static void BindDefault() noexcept;

		void BindColorTexture(
			std::uint32_t textureUnit) const noexcept;

		[[nodiscard]] std::uint32_t Width()
			const noexcept;

		[[nodiscard]] std::uint32_t Height()
			const noexcept;

	private:
		void CreateAttachments(
			std::uint32_t width,
			std::uint32_t height);

		void DestroyAttachments() noexcept;

		unsigned int m_Framebuffer = 0;
		unsigned int m_ColorTexture = 0;
		unsigned int m_DepthRenderbuffer = 0;

		std::uint32_t m_Width = 0;
		std::uint32_t m_Height = 0;
	};
}