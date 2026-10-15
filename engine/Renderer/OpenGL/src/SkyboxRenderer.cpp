#include <ForgeSim/Renderer/OpenGL/SkyboxRenderer.hpp>

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>

#include <glad/gl.h>

#include <ForgeSim/Renderer/OpenGL/TextureCube.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		namespace SkyboxShaderUniform
		{
			constexpr std::string_view view =
				"uView";

			constexpr std::string_view projection =
				"uProjection";

			constexpr std::string_view environmentMap =
				"uEnvironmentMap";

			constexpr std::string_view environmentIntensity =
				"uEnvironmentIntensity";

			constexpr std::array required{
				view,
				projection,
				environmentMap,
				environmentIntensity
			};
		}

		constexpr std::uint32_t environmentTextureUnit = 0;

		constexpr std::array<float, 24> cubeVertices{
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f
		};

		constexpr std::array<std::uint32_t, 36>
			cubeIndices{
				0, 2, 1,
				0, 3, 2,

				4, 5, 6,
				4, 6, 7,

				0, 4, 7,
				0, 7, 3,

				1, 2, 6,
				1, 6, 5,

				0, 1, 5,
				0, 5, 4,

				3, 7, 6,
				3, 6, 2
		};
	}

	SkyboxRenderer::SkyboxRenderer(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
		: m_ShaderProgram(
			vertexShaderSource,
			fragmentShaderSource,
			SkyboxShaderUniform::required)
		, m_VertexBuffer(
			cubeVertices.data(),
			cubeVertices.size() *
			sizeof(float))
		, m_IndexBuffer(
			cubeIndices.data(),
			cubeIndices.size() *
			sizeof(std::uint32_t))
		, m_VertexArray()
	{
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_VertexArray.SetVertexBuffer(
			m_VertexBuffer,
			vertexBindingIndex,
			3 * sizeof(float));

		m_VertexArray.SetIndexBuffer(
			m_IndexBuffer);

		m_VertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			0);
	}

	void SkyboxRenderer::Render(
		const TextureCube& environmentMap,
		const glm::mat4& view,
		const glm::mat4& projection,
		float intensity)
	{
		GLint previousDepthFunction = GL_LESS;
		GLboolean previousDepthWriteMask = GL_TRUE;

		glGetIntegerv(
			GL_DEPTH_FUNC,
			&previousDepthFunction);

		glGetBooleanv(
			GL_DEPTH_WRITEMASK,
			&previousDepthWriteMask);

		const GLboolean depthTestWasEnabled =
			glIsEnabled(GL_DEPTH_TEST);

		const GLboolean cullFaceWasEnabled =
			glIsEnabled(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		m_ShaderProgram.Bind();

		m_ShaderProgram.SetMatrix4x4(
			SkyboxShaderUniform::view,
			view);

		m_ShaderProgram.SetMatrix4x4(
			SkyboxShaderUniform::projection,
			projection);

		m_ShaderProgram.SetFloat(
			SkyboxShaderUniform::environmentIntensity,
			intensity);

		m_ShaderProgram.SetFloat(
			SkyboxShaderUniform::environmentIntensity,
			intensity);

		// Remove any material sampler left bound to this unit so the
		// cubemap uses its own filtering and wrapping configuration.
		glBindSampler(
			environmentTextureUnit,
			0);

		environmentMap.Bind(
			environmentTextureUnit);

		m_ShaderProgram.SetInt(
			SkyboxShaderUniform::environmentMap,
			static_cast<int>(
				environmentTextureUnit));

		m_VertexArray.Bind();

		environmentMap.Bind(
			environmentTextureUnit);

		m_ShaderProgram.SetInt(
			SkyboxShaderUniform::environmentMap,
			static_cast<int>(
				environmentTextureUnit));

		m_VertexArray.Bind();

		glDrawElements(
			GL_TRIANGLES,
			static_cast<GLsizei>(
				cubeIndices.size()),
			GL_UNSIGNED_INT,
			nullptr);

		glDepthMask(previousDepthWriteMask);

		glDepthFunc(
			static_cast<GLenum>(
				previousDepthFunction));

		if (depthTestWasEnabled == GL_TRUE)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}

		if (cullFaceWasEnabled == GL_TRUE)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}

	void SkyboxRenderer::ReloadShaders(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
	{
		ShaderProgram replacement{
			vertexShaderSource,
			fragmentShaderSource,
			SkyboxShaderUniform::required
		};

		m_ShaderProgram =
			std::move(replacement);
	}
}