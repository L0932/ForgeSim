#include <ForgeSim/Renderer/OpenGL/ColorRenderTarget.hpp>
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

            constexpr std::string_view applyToneMapping = "uApplyToneMapping";

			constexpr std::array required{
				hdrTexture,
				exposure,
                applyToneMapping
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
		float exposure,
        bool applyToneMapping)
	{
		HdrRenderTarget::BindDefault();

		glViewport(
			0,
			0,
			static_cast<GLsizei>(source.Width()),
			static_cast<GLsizei>(source.Height()));

		Draw(source, exposure, applyToneMapping);
	}

	void ToneMappingRenderer::RenderToTarget(
		const HdrRenderTarget& source,
		ColorRenderTarget& destination,
		float exposure,
        bool applyToneMapping)
	{
		destination.Resize(source.Width(), source.Height());
		destination.Bind();

		glViewport(
			0,
			0,
			static_cast<GLsizei>(destination.Width()),
			static_cast<GLsizei>(destination.Height()));

		Draw(source, exposure, applyToneMapping);

		HdrRenderTarget::BindDefault();
	}

	void ToneMappingRenderer::Draw(
		const HdrRenderTarget& source,
		float exposure,
        bool applyToneMapping)
	{
		const GLboolean depthTestWasEnabled =
			glIsEnabled(GL_DEPTH_TEST);

		const GLboolean framebufferSrgbWasEnabled =
			glIsEnabled(GL_FRAMEBUFFER_SRGB);

		glDisable(GL_DEPTH_TEST);

		// The tone-mapping shader already converts linear color to sRGB.
		glDisable(GL_FRAMEBUFFER_SRGB);

		source.BindColorTexture(hdrTextureUnit);
		m_Sampler.Bind(hdrTextureUnit);

		m_ShaderProgram.Bind();

		m_ShaderProgram.SetInt(
			ToneMappingShaderUniform::hdrTexture,
			static_cast<int>(hdrTextureUnit));

		m_ShaderProgram.SetFloat(
			ToneMappingShaderUniform::exposure,
			exposure);

        m_ShaderProgram.SetInt(ToneMappingShaderUniform::applyToneMapping, applyToneMapping ? 1 : 0);

		m_VertexArray.Bind();

		glDrawArrays(GL_TRIANGLES, 0, 3);

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