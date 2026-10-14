#include <ForgeSim/Renderer/OpenGL/ToneMappingRenderer.hpp>

#include <array>
#include <string_view>
#include <utility>

#include <glad/gl.h>

#include <ForgeSim/Renderer/OpenGL/HdrRenderTarget.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		namespace ToneMappingShaderUniform
		{
			constexpr std::string_view hdrTexture =
				"uHdrTexture";

			constexpr std::string_view exposure =
				"uExposure";

			constexpr std::array required{
				hdrTexture,
				exposure
			};
		}

		constexpr std::uint32_t hdrTextureUnit = 0;

		[[nodiscard]]
		SamplerSpecification CreateToneMappingSampler()
		{
			return SamplerSpecification{
				.minificationFilter =
					MinificationFilter::Linear,

				.magnificationFilter =
					MagnificationFilter::Linear,

				.wrapU = SamplerWrap::ClampToEdge,
				.wrapV = SamplerWrap::ClampToEdge
			};
		}
	}

	ToneMappingRenderer::ToneMappingRenderer(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
		: m_ShaderProgram(
			vertexShaderSource,
			fragmentShaderSource,
			ToneMappingShaderUniform::required)
		, m_Sampler(
			CreateToneMappingSampler())
		, m_VertexArray()
	{}

	void ToneMappingRenderer::RenderToDefaultFramebuffer(
		const HdrRenderTarget& source,
		float exposure)
	{
		HdrRenderTarget::BindDefault();

		glViewport(
			0,
			0,
			static_cast<GLsizei>(source.Width()),
			static_cast<GLsizei>(source.Height()));

		const GLboolean depthTestWasEnabled =
			glIsEnabled(GL_DEPTH_TEST);

		const GLboolean framebufferSrgbWasEnabled =
			glIsEnabled(GL_FRAMEBUFFER_SRGB);

		glDisable(GL_DEPTH_TEST);

		// The shader performs the linear-to-sRGB conversion.
		glDisable(GL_FRAMEBUFFER_SRGB);

		source.BindColorTexture(
			hdrTextureUnit);

		m_Sampler.Bind(
			hdrTextureUnit);

		m_ShaderProgram.Bind();

		m_ShaderProgram.SetInt(
			ToneMappingShaderUniform::hdrTexture,
			static_cast<int>(hdrTextureUnit));

		m_ShaderProgram.SetFloat(
			ToneMappingShaderUniform::exposure,
			exposure);

		m_VertexArray.Bind();

		glDrawArrays(
			GL_TRIANGLES,
			0,
			3);

		if (framebufferSrgbWasEnabled == GL_TRUE)
		{
			glEnable(GL_FRAMEBUFFER_SRGB);
		}

		if (depthTestWasEnabled == GL_TRUE)
		{
			glEnable(GL_DEPTH_TEST);
		}
	}

	void ToneMappingRenderer::ReloadShaders(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
	{
		ShaderProgram replacement{
			vertexShaderSource,
			fragmentShaderSource,
			ToneMappingShaderUniform::required
		};

		m_ShaderProgram =
			std::move(replacement);
	}
}