#pragma once

#include <string_view>

#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class HdrRenderTarget;
	class ColorRenderTarget;

	class ToneMappingRenderer final
	{
	public:
		ToneMappingRenderer(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

        // Disabling tone mapping preserves exposure and sRGB conversion, with
        // clipping to the SDR display range instead of the ACES approximation.
		void RenderToDefaultFramebuffer(
			const HdrRenderTarget& source,
			float exposure,
            bool applyToneMapping = true);

		void ReloadShaders(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

		void RenderToTarget(
			const HdrRenderTarget& source,
			ColorRenderTarget& destination,
			float exposure,
            bool applyToneMapping = true);

	private:
		void Draw(
			const HdrRenderTarget& source,
			float exposure,
            bool applyToneMapping);

		ShaderProgram m_ShaderProgram;
		Sampler m_Sampler;
		VertexArray m_VertexArray;
	};
}