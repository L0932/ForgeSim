#include <ForgeSim/Renderer/OpenGL/EnvironmentMapProcessor.hpp>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include <glad/gl.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		namespace CaptureShaderUniform
		{
			constexpr std::string_view projection =
				"uProjection";

			constexpr std::string_view view =
				"uView";
		}

		namespace ConversionShaderUniform
		{
			constexpr std::string_view equirectangularMap =
				"uEquirectangularMap";

			constexpr std::array required{
				CaptureShaderUniform::projection,
				CaptureShaderUniform::view,
				equirectangularMap
			};
		}

		namespace IrradianceShaderUniform
		{
			constexpr std::string_view environmentMap =
				"uEnvironmentMap";

			constexpr std::array required{
				CaptureShaderUniform::projection,
				CaptureShaderUniform::view,
				environmentMap
			};
		}

		constexpr std::uint32_t sourceTextureUnit = 0;

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

		[[nodiscard]]
		SamplerSpecification CreateSourceSampler()
		{
			return SamplerSpecification{
				.minificationFilter =
					MinificationFilter::Linear,

				.magnificationFilter =
					MagnificationFilter::Linear,

				.wrapU = SamplerWrap::Repeat,
				.wrapV = SamplerWrap::ClampToEdge
			};
		}

		[[nodiscard]]
		std::array<glm::mat4, 6> CreateCaptureViews()
		{
			const glm::vec3 origin{ 0.0f };

			return {
				glm::lookAt(
					origin,
					glm::vec3{ 1.0f, 0.0f, 0.0f },
					glm::vec3{ 0.0f, -1.0f, 0.0f }),

				glm::lookAt(
					origin,
					glm::vec3{ -1.0f, 0.0f, 0.0f },
					glm::vec3{ 0.0f, -1.0f, 0.0f }),

				glm::lookAt(
					origin,
					glm::vec3{ 0.0f, 1.0f, 0.0f },
					glm::vec3{ 0.0f, 0.0f, 1.0f }),

				glm::lookAt(
					origin,
					glm::vec3{ 0.0f, -1.0f, 0.0f },
					glm::vec3{ 0.0f, 0.0f, -1.0f }),

				glm::lookAt(
					origin,
					glm::vec3{ 0.0f, 0.0f, 1.0f },
					glm::vec3{ 0.0f, -1.0f, 0.0f }),

				glm::lookAt(
					origin,
					glm::vec3{ 0.0f, 0.0f, -1.0f },
					glm::vec3{ 0.0f, -1.0f, 0.0f })
			};
		}
	}

	EnvironmentMapProcessor::EnvironmentMapProcessor(
		std::string_view captureVertexShaderSource,
		std::string_view conversionFragmentShaderSource,
		std::string_view irradianceFragmentShaderSource)
		: m_ConversionShaderProgram(
			captureVertexShaderSource,
			conversionFragmentShaderSource,
			ConversionShaderUniform::required)
		, m_IrradianceShaderProgram(
			captureVertexShaderSource,
			irradianceFragmentShaderSource,
			IrradianceShaderUniform::required)
		, m_CubeVertexBuffer(
			cubeVertices.data(),
			cubeVertices.size() *
			sizeof(float))
		, m_CubeIndexBuffer(
			cubeIndices.data(),
			cubeIndices.size() *
			sizeof(std::uint32_t))
		, m_CubeVertexArray()
		, m_EquirectangularSampler(
			CreateSourceSampler())
	{
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_CubeVertexArray.SetVertexBuffer(
			m_CubeVertexBuffer,
			vertexBindingIndex,
			3 * sizeof(float));

		m_CubeVertexArray.SetIndexBuffer(
			m_CubeIndexBuffer);

		m_CubeVertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			0);

		glCreateFramebuffers(
			1,
			&m_Framebuffer);

		if (m_Framebuffer == 0)
		{
			throw std::runtime_error(
				"Failed to create the environment capture framebuffer.");
		}

		glCreateRenderbuffers(
			1,
			&m_DepthRenderbuffer);

		if (m_DepthRenderbuffer == 0)
		{
			glDeleteFramebuffers(
				1,
				&m_Framebuffer);

			m_Framebuffer = 0;

			throw std::runtime_error(
				"Failed to create the environment capture depth buffer.");
		}

		glNamedFramebufferRenderbuffer(
			m_Framebuffer,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			m_DepthRenderbuffer);

		glNamedFramebufferDrawBuffer(
			m_Framebuffer,
			GL_COLOR_ATTACHMENT0);
	}

	EnvironmentMapProcessor::~EnvironmentMapProcessor()
	{
		if (m_DepthRenderbuffer != 0)
		{
			glDeleteRenderbuffers(
				1,
				&m_DepthRenderbuffer);
		}

		if (m_Framebuffer != 0)
		{
			glDeleteFramebuffers(
				1,
				&m_Framebuffer);
		}
	}

	void EnvironmentMapProcessor::CaptureCubemapFaces(
		TextureCube& destination,
		std::uint32_t faceSize,
		ShaderProgram& shaderProgram)
	{
		GLint previousDrawFramebuffer = 0;
		std::array<GLint, 4> previousViewport{};
		GLint previousDepthFunction = GL_LESS;

		glGetIntegerv(
			GL_DRAW_FRAMEBUFFER_BINDING,
			&previousDrawFramebuffer);

		glGetIntegerv(
			GL_VIEWPORT,
			previousViewport.data());

		glGetIntegerv(
			GL_DEPTH_FUNC,
			&previousDepthFunction);

		const GLboolean depthTestWasEnabled =
			glIsEnabled(GL_DEPTH_TEST);

		const GLboolean cullFaceWasEnabled =
			glIsEnabled(GL_CULL_FACE);

		const auto restoreState = [&]()
			{
				glBindFramebuffer(
					GL_DRAW_FRAMEBUFFER,
					static_cast<GLuint>(
						previousDrawFramebuffer));

				glViewport(
					previousViewport[0],
					previousViewport[1],
					previousViewport[2],
					previousViewport[3]);

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
			};

		try
		{
			glNamedRenderbufferStorage(
				m_DepthRenderbuffer,
				GL_DEPTH_COMPONENT24,
				static_cast<GLsizei>(faceSize),
				static_cast<GLsizei>(faceSize));

			glBindFramebuffer(
				GL_DRAW_FRAMEBUFFER,
				m_Framebuffer);

			glViewport(
				0,
				0,
				static_cast<GLsizei>(faceSize),
				static_cast<GLsizei>(faceSize));

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);

			const glm::mat4 captureProjection =
				glm::perspective(
					glm::radians(90.0f),
					1.0f,
					0.1f,
					10.0f);

			shaderProgram.SetMatrix4x4(
				CaptureShaderUniform::projection,
				captureProjection);

			const auto captureViews =
				CreateCaptureViews();

			m_CubeVertexArray.Bind();

			for (std::size_t faceIndex = 0;
				faceIndex < captureViews.size();
				++faceIndex)
			{
				destination.AttachFaceToBoundFramebuffer(
					static_cast<std::uint32_t>(
						faceIndex),
					0);

				if (glCheckFramebufferStatus(
					GL_DRAW_FRAMEBUFFER) !=
					GL_FRAMEBUFFER_COMPLETE)
				{
					throw std::runtime_error(
						"Environment capture framebuffer "
						"is incomplete.");
				}

				shaderProgram.SetMatrix4x4(
					CaptureShaderUniform::view,
					captureViews[faceIndex]);

				glClear(
					GL_COLOR_BUFFER_BIT |
					GL_DEPTH_BUFFER_BIT);

				glDrawElements(
					GL_TRIANGLES,
					static_cast<GLsizei>(
						cubeIndices.size()),
					GL_UNSIGNED_INT,
					nullptr);
			}
		}
		catch (...)
		{
			restoreState();
			throw;
		}

		restoreState();
	}

	TextureCube
		EnvironmentMapProcessor::
		ConvertEquirectangularToCubemap(
			const Texture2D& equirectangularTexture,
			std::uint32_t cubemapSize)
	{
		if (cubemapSize == 0)
		{
			throw std::invalid_argument(
				"Environment cubemap size must be positive.");
		}

		const std::uint32_t mipLevelCount =
			std::bit_width(cubemapSize);

		TextureCube environmentCubemap{
			cubemapSize,
			mipLevelCount
		};

		equirectangularTexture.Bind(
			sourceTextureUnit);

		m_EquirectangularSampler.Bind(
			sourceTextureUnit);

		m_ConversionShaderProgram.Bind();

		m_ConversionShaderProgram.SetInt(
			ConversionShaderUniform::equirectangularMap,
			static_cast<int>(
				sourceTextureUnit));

		CaptureCubemapFaces(
			environmentCubemap,
			cubemapSize,
			m_ConversionShaderProgram);

		environmentCubemap.GenerateMipmaps();

		return environmentCubemap;
	}

	TextureCube
		EnvironmentMapProcessor::
		CreateDiffuseIrradianceMap(
			const TextureCube& environmentCubemap,
			std::uint32_t irradianceMapSize)
	{
		if (irradianceMapSize == 0)
		{
			throw std::invalid_argument(
				"Diffuse irradiance map size must be positive.");
		}

		TextureCube irradianceMap{
			irradianceMapSize,
			1
		};

		glBindSampler(
			sourceTextureUnit,
			0);

		environmentCubemap.Bind(
			sourceTextureUnit);

		m_IrradianceShaderProgram.Bind();

		m_IrradianceShaderProgram.SetInt(
			IrradianceShaderUniform::environmentMap,
			static_cast<int>(
				sourceTextureUnit));

		CaptureCubemapFaces(
			irradianceMap,
			irradianceMapSize,
			m_IrradianceShaderProgram);

		return irradianceMap;
	}
}