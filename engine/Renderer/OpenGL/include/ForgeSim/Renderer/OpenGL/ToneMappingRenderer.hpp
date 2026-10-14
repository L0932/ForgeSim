#pragma once

#include <string_view>

#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class HdrRenderTarget;

	class ToneMappingRenderer final
	{
	public:
		ToneMappingRenderer(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

		void RenderToDefaultFramebuffer(
			const HdrRenderTarget& source,
			float exposure);

		void ReloadShaders(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

	private:
		ShaderProgram m_ShaderProgram;
		Sampler m_Sampler;
		VertexArray m_VertexArray;
	};
}