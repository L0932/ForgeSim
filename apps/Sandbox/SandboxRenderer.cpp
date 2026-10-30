#include "LightDebugLayout.hpp"
#include "LightSubmission.hpp"
#include "SandboxPaths.hpp"
#include "SandboxRenderer.hpp"
#include "SceneBounds.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/ImageLoader.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Renderer/DirectionalShadowProjection.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{

        namespace DirectionalShadowUniform
        {
            constexpr std::string_view model = "uModel";
            constexpr std::string_view lightViewProjection = "uLightViewProjection";
            constexpr std::string_view baseColorTexture = "uBaseColorTexture";
            constexpr std::string_view baseColorAlpha = "uBaseColorAlpha";
            constexpr std::string_view alphaMaskEnabled = "uAlphaMaskEnabled";
            constexpr std::string_view alphaCutoff = "uAlphaCutoff";

            constexpr std::array required{
                model, lightViewProjection, baseColorTexture,
                baseColorAlpha, alphaMaskEnabled, alphaCutoff
            };
        }

        // Reserved for the shadow pass's material-alpha sampling.
        // Units 0-7 are currently used by the visible scene pass.
        constexpr std::uint32_t shadowAlphaTextureUnit = 8;

        // Restore the GL state changed by the depth pass, including on error.
        class DirectionalShadowPassState final
        {
        public:
            DirectionalShadowPassState()
            {
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_DrawFramebuffer);
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_ReadFramebuffer);
                glGetIntegerv(GL_VIEWPORT, m_Viewport.data());
                glGetIntegerv(GL_CURRENT_PROGRAM, &m_Program);
                glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_VertexArray);
                glGetIntegerv(GL_DEPTH_FUNC, &m_DepthFunction);
                glGetBooleanv(GL_DEPTH_WRITEMASK, &m_DepthWriteMask);
                glGetDoublev(GL_DEPTH_RANGE, m_DepthRange.data());
                glGetIntegerv(GL_CULL_FACE_MODE, &m_CullFaceMode);

				glGetFloatv(
					GL_POLYGON_OFFSET_FACTOR,
					&m_PolygonOffsetFactor);

				glGetFloatv(
					GL_POLYGON_OFFSET_UNITS,
					&m_PolygonOffsetUnits);

                for (std::size_t i = 0; i < m_Capabilities.size(); ++i)
                {
                    m_Enabled[i] = glIsEnabled(m_Capabilities[i]);
                }
            }

            ~DirectionalShadowPassState()
            {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                    static_cast<GLuint>(m_DrawFramebuffer));
                glBindFramebuffer(GL_READ_FRAMEBUFFER,
                    static_cast<GLuint>(m_ReadFramebuffer));
                glViewport(m_Viewport[0], m_Viewport[1],
                    m_Viewport[2], m_Viewport[3]);
                glUseProgram(static_cast<GLuint>(m_Program));
                glBindVertexArray(static_cast<GLuint>(m_VertexArray));
                glDepthFunc(static_cast<GLenum>(m_DepthFunction));
                glDepthMask(m_DepthWriteMask);
                glDepthRange(m_DepthRange[0], m_DepthRange[1]);
                glCullFace(static_cast<GLenum>(m_CullFaceMode));

				glPolygonOffset(
					m_PolygonOffsetFactor,
					m_PolygonOffsetUnits);

                for (std::size_t i = 0; i < m_Capabilities.size(); ++i)
                {
                    if (m_Enabled[i] == GL_TRUE)
                    {
                        glEnable(m_Capabilities[i]);
                    }
                    else
                    {
                        glDisable(m_Capabilities[i]);
                    }
                }
            }

            DirectionalShadowPassState(const DirectionalShadowPassState&) = delete;
            DirectionalShadowPassState& operator=(const DirectionalShadowPassState&) = delete;

        private:
            static constexpr std::array<GLenum, 6> m_Capabilities{
                GL_DEPTH_TEST, GL_CULL_FACE, GL_BLEND,
                GL_SCISSOR_TEST, GL_POLYGON_OFFSET_FILL, GL_DEPTH_CLAMP
            };
            std::array<GLboolean, 6> m_Enabled{};
            std::array<GLint, 4> m_Viewport{};
            std::array<GLdouble, 2> m_DepthRange{};
            GLint m_DrawFramebuffer = 0;
            GLint m_ReadFramebuffer = 0;
            GLint m_Program = 0;
            GLint m_VertexArray = 0;
            GLint m_DepthFunction = GL_LESS;
            GLint m_CullFaceMode = GL_BACK;
            GLboolean m_DepthWriteMask = GL_TRUE;

			GLfloat m_PolygonOffsetFactor = 0.0f;
			GLfloat m_PolygonOffsetUnits = 0.0f;
        };

		namespace SandboxRendererShaderUniform
		{
			constexpr std::string_view model =
				"uModel";

			constexpr std::string_view view =
				"uView";

			constexpr std::string_view projection =
				"uProjection";

			constexpr std::string_view baseColorFactor =
				"uBaseColorFactor";

			constexpr std::string_view baseColorTexture =
				"uBaseColorTexture";

			constexpr std::string_view alphaMaskEnabled =
				"uAlphaMaskEnabled";

			constexpr std::string_view alphaCutoff =
				"uAlphaCutoff";

			constexpr std::string_view doubleSided =
				"uDoubleSided";

			constexpr std::string_view metallicRoughnessTexture =
				"uMetallicRoughnessTexture";

			constexpr std::string_view normalTexture =
				"uNormalTexture";

			constexpr std::string_view occlusionTexture =
				"uOcclusionTexture";

			constexpr std::string_view emissiveTexture =
				"uEmissiveTexture";

			constexpr std::string_view normalScale =
				"uNormalScale";

			constexpr std::string_view occlusionStrength =
				"uOcclusionStrength";

			constexpr std::string_view emissiveFactor =
				"uEmissiveFactor";

			constexpr std::string_view cameraPosition =
				"uCameraPosition";

			constexpr std::string_view lightDirection =
				"uLightDirection";

			constexpr std::string_view lightRadiance =
				"uLightRadiance";

			constexpr std::string_view metallicFactor =
				"uMetallicFactor";

			constexpr std::string_view roughnessFactor =
				"uRoughnessFactor";

			constexpr std::string_view selectionAmount =
				"uSelectionAmount";

			constexpr std::string_view selectionColor =
				"uSelectionColor";

			constexpr std::string_view diffuseIrradianceMap =
				"uDiffuseIrradianceMap";

			constexpr std::string_view environmentIntensity =
				"uEnvironmentIntensity";

			constexpr std::string_view prefilteredSpecularMap =
				"uPrefilteredSpecularMap";

			constexpr std::string_view brdfLookupTexture =
				"uBrdfLookupTexture";

			constexpr std::string_view maximumSpecularLod =
				"uMaximumSpecularLod";

			constexpr std::string_view pointLightCount =
				"uPointLightCount";

			constexpr std::string_view spotLightCount =
				"uSpotLightCount";

			constexpr std::string_view directionalShadowMap =
				"uDirectionalShadowMap";

			constexpr std::string_view directionalLightViewProjection =
				"uDirectionalLightViewProjection";

			constexpr std::string_view directionalShadowsEnabled =
				"uDirectionalShadowsEnabled";

			constexpr std::string_view directionalShadowFilteringEnabled =
				"uDirectionalShadowFilteringEnabled";

			constexpr std::string_view directionalShadowMinimumBias =
				"uDirectionalShadowMinimumBias";

			constexpr std::string_view directionalShadowSlopeBias =
				"uDirectionalShadowSlopeBias";

			constexpr std::array required{
				model,
				view,
				projection,
				baseColorFactor,
				baseColorTexture,
				alphaMaskEnabled,
				alphaCutoff,
				doubleSided,
				metallicRoughnessTexture,
				normalTexture,
				occlusionTexture,
				emissiveTexture,
				normalScale,
				occlusionStrength,
				emissiveFactor,
				cameraPosition,
				lightDirection,
				lightRadiance,
				metallicFactor,
				roughnessFactor,
				selectionAmount,
				selectionColor,
				diffuseIrradianceMap,
				environmentIntensity,
				prefilteredSpecularMap,
				brdfLookupTexture,
				maximumSpecularLod,
				pointLightCount,
				spotLightCount,
				directionalShadowMap,
				directionalLightViewProjection,
				directionalShadowsEnabled,
				directionalShadowFilteringEnabled,
				directionalShadowMinimumBias,
				directionalShadowSlopeBias,
			};

		}

		constexpr std::array<std::uint8_t, 4>
			fallbackWhiteTexturePixels{
				255,
				255,
				255,
				255
		};

		constexpr std::array<std::uint8_t, 4>
			fallbackNormalTexturePixels{
				128,
				128,
				255,
				255
		};

		constexpr std::array<float, 3>
			fallbackEnvironmentPixels{
				0.03f,
				0.03f,
				0.03f
		};

		constexpr std::uint32_t baseColorTextureUnit = 0;
		constexpr std::uint32_t metallicRoughnessTextureUnit = 1;
		constexpr std::uint32_t normalTextureUnit = 2;
		constexpr std::uint32_t occlusionTextureUnit = 3;
		constexpr std::uint32_t emissiveTextureUnit = 4;
		constexpr std::uint32_t diffuseIrradianceTextureUnit = 5;
		constexpr std::uint32_t prefilteredSpecularTextureUnit = 6;
		constexpr std::uint32_t brdfLookupTextureUnit = 7;

		[[nodiscard]] std::string LoadTextFile(
			const std::filesystem::path& path)
		{
			std::ifstream file{
				path,
				std::ios::in | std::ios::binary
			};

			if (!file)
			{
				throw std::runtime_error(
					"Failed to open text file: " +
					path.string());
			}

			std::ostringstream contents;
			contents << file.rdbuf();

			if (file.bad())
			{
				throw std::runtime_error(
					"Failed while reading text file: " +
					path.string());
			}

			return contents.str();
		}

		[[nodiscard]] std::filesystem::path ShaderPath(
			const std::filesystem::path& filename)
		{
			return ShaderDirectory() / filename;
		}

		[[nodiscard]] std::string LoadShaderSource(
			const std::filesystem::path& filename)
		{
			return LoadTextFile(
				ShaderPath(filename));
		}

		[[nodiscard]]
		ForgeSim::Renderer::OpenGL::ShaderProgram CreateShaderProgram(
			const std::filesystem::path& vertexShaderFilename,
			const std::filesystem::path& fragmentShaderFilename,
			std::span<const std::string_view>
			requiredUniformNames)
		{
			const std::string vertexShaderSource =
				LoadShaderSource(vertexShaderFilename);

			const std::string fragmentShaderSource =
				LoadShaderSource(fragmentShaderFilename);

			return ForgeSim::Renderer::OpenGL::ShaderProgram{
				vertexShaderSource,
				fragmentShaderSource,
				requiredUniformNames
			};
		}

		[[nodiscard]] std::filesystem::path ResolveObjectModelPath(
			const std::filesystem::path& filename)
		{
			const std::filesystem::path modelPath =
				filename.is_absolute()
				? filename
				: BundledAssetDirectory() / "models" / filename;

			return std::filesystem::weakly_canonical(
				std::filesystem::absolute(modelPath));
		}

		[[nodiscard]]
		Renderer::OpenGL::MinificationFilter
			ToMinificationFilter(
				std::optional<Assets::TextureFilter> filter)
		{
			if (!filter)
			{
				return Renderer::OpenGL::
					MinificationFilter::LinearMipmapLinear;
			}

			using Source = Assets::TextureFilter;
			using Destination =
				Renderer::OpenGL::MinificationFilter;

			switch (*filter)
			{
			case Source::Nearest:
				return Destination::Nearest;

			case Source::Linear:
				return Destination::Linear;

			case Source::NearestMipmapNearest:
				return Destination::NearestMipmapNearest;

			case Source::LinearMipmapNearest:
				return Destination::LinearMipmapNearest;

			case Source::NearestMipmapLinear:
				return Destination::NearestMipmapLinear;

			case Source::LinearMipmapLinear:
				return Destination::LinearMipmapLinear;
			}

			throw std::logic_error(
				"Unsupported imported minification filter.");
		}

		[[nodiscard]]
		Renderer::OpenGL::MagnificationFilter
			ToMagnificationFilter(
				std::optional<Assets::TextureFilter> filter)
		{
			if (!filter)
			{
				return Renderer::OpenGL::
					MagnificationFilter::Linear;
			}

			switch (*filter)
			{
			case Assets::TextureFilter::Nearest:
				return Renderer::OpenGL::
					MagnificationFilter::Nearest;

			case Assets::TextureFilter::Linear:
				return Renderer::OpenGL::
					MagnificationFilter::Linear;

			default:
				throw std::logic_error(
					"Magnification filters cannot use mipmaps.");
			}
		}

		[[nodiscard]]
		Renderer::OpenGL::SamplerWrap ToSamplerWrap(
			Assets::TextureWrap wrap)
		{
			switch (wrap)
			{
			case Assets::TextureWrap::Repeat:
				return Renderer::OpenGL::SamplerWrap::Repeat;

			case Assets::TextureWrap::MirroredRepeat:
				return Renderer::OpenGL::
					SamplerWrap::MirroredRepeat;

			case Assets::TextureWrap::ClampToEdge:
				return Renderer::OpenGL::
					SamplerWrap::ClampToEdge;
			}

			throw std::logic_error(
				"Unsupported imported sampler wrapping mode.");
		}

		[[nodiscard]]
		Renderer::OpenGL::SamplerSpecification
			CreateSamplerSpecification(
				const Assets::SamplerData& sampler)
		{
			return Renderer::OpenGL::SamplerSpecification{
				.minificationFilter =
					ToMinificationFilter(
						sampler.minificationFilter),

				.magnificationFilter =
					ToMagnificationFilter(
						sampler.magnificationFilter),

				.wrapU = ToSamplerWrap(sampler.wrapU),
				.wrapV = ToSamplerWrap(sampler.wrapV)
			};
		}

		[[nodiscard]] std::filesystem::path EnvironmentPath(
			const std::filesystem::path& filename)
		{
			return filename.is_absolute()
				? filename
				: BundledAssetDirectory() / "environments" / filename;
		}

		[[nodiscard]]
		Renderer::OpenGL::Texture2D LoadEnvironmentTexture(
			const std::optional<std::filesystem::path>& filename)
		{
			if (!filename)
			{
				return Renderer::OpenGL::Texture2D{
					1,
					1,
					fallbackEnvironmentPixels
				};
			}

			const Assets::HdrImageData image =
				Assets::LoadHdrImage(
					EnvironmentPath(*filename));

			if (!image.IsValid())
			{
				throw std::runtime_error(
					"Cannot upload an invalid HDR environment image.");
			}

			return Renderer::OpenGL::Texture2D{
				image.width,
				image.height,
				image.pixels
			};
		}

		[[nodiscard]] glm::mat4 CreateSceneNodeLocalMatrix(
			const Assets::SceneNodeTransform& transform)
		{
			if (transform.matrix)
			{
				// Both glTF and GLM store matrix components in
				// column-major order.
				return glm::make_mat4(
					transform.matrix->data());
			}

			const glm::vec3 translation{
				transform.translation[0],
				transform.translation[1],
				transform.translation[2]
			};

			// glTF stores quaternion components as X, Y, Z, W,
			// while GLM's constructor accepts W, X, Y, Z.
			const glm::quat rotation{
				transform.rotation[3],
				transform.rotation[0],
				transform.rotation[1],
				transform.rotation[2]
			};

			const glm::vec3 scale{
				transform.scale[0],
				transform.scale[1],
				transform.scale[2]
			};

			return
				glm::translate(
					glm::mat4{ 1.0f },
					translation) *
				glm::mat4_cast(
					glm::normalize(rotation)) *
				glm::scale(
					glm::mat4{ 1.0f },
					scale);
		}
	}

	void ExpandSceneNodeBounds(
		const Assets::SceneAsset& scene,
		Assets::SceneNodeIndex nodeIndex,
		const glm::mat4& parentTransform,
		glm::vec3& minimum,
		glm::vec3& maximum,
		bool& containsVertices)
	{
		if (nodeIndex >= scene.nodes.size())
		{
			throw std::runtime_error(
				"Scene bounds reference an invalid node.");
		}

		const Assets::SceneNodeData& node =
			scene.nodes[nodeIndex];

		const glm::mat4 nodeTransform =
			parentTransform *
			CreateSceneNodeLocalMatrix(
				node.localTransform);

		if (node.meshIndex)
		{
			if (*node.meshIndex >= scene.meshes.size())
			{
				throw std::runtime_error(
					"Scene node references an invalid mesh.");
			}

			const Assets::SceneMeshData& mesh =
				scene.meshes[*node.meshIndex];

			for (const Assets::MeshPrimitiveData& primitive :
				mesh.primitives)
			{
				for (const Assets::MeshVertex& vertex :
					primitive.geometry.vertices)
				{
					const glm::vec4 transformedPosition =
						nodeTransform *
						glm::vec4{
							vertex.position[0],
							vertex.position[1],
							vertex.position[2],
							1.0f
					};

					const glm::vec3 position{
						transformedPosition
					};

					minimum = glm::min(
						minimum,
						position);

					maximum = glm::max(
						maximum,
						position);

					containsVertices = true;
				}
			}
		}

		for (const Assets::SceneNodeIndex childIndex :
		node.childIndices)
		{
			ExpandSceneNodeBounds(
				scene,
				childIndex,
				nodeTransform,
				minimum,
				maximum,
				containsVertices);
		}
	}

	[[nodiscard]]
	Renderer::AxisAlignedBoundingBox CalculateSceneBounds(
		const Assets::SceneAsset& scene)
	{
		glm::vec3 minimum{
			std::numeric_limits<float>::max()
		};

		glm::vec3 maximum{
			std::numeric_limits<float>::lowest()
		};

		bool containsVertices = false;

		for (const Assets::SceneNodeIndex rootNodeIndex :
		scene.rootNodeIndices)
		{
			ExpandSceneNodeBounds(
				scene,
				rootNodeIndex,
				glm::mat4{ 1.0f },
				minimum,
				maximum,
				containsVertices);
		}

		if (!containsVertices)
		{
			throw std::runtime_error(
				"Cannot calculate bounds for a scene "
				"without vertices.");
		}

		return Renderer::AxisAlignedBoundingBox{
			.minimum = minimum,
			.maximum = maximum
		};
	}

	SandboxRenderer::SandboxRenderer()
		: SandboxRenderer(
			SandboxRendererSpecification{})
	{}

	SandboxRenderer::SandboxRenderer(
		const SandboxRendererSpecification& specification)
        : m_DirectionalShadowShaderMonitor(
            ShaderPath("DirectionalShadow.vert"),
            ShaderPath("DirectionalShadow.frag"))
        , m_DirectionalShadowShaderProgram(
            CreateShaderProgram(
                m_DirectionalShadowShaderMonitor.VertexShaderPath(),
                m_DirectionalShadowShaderMonitor.FragmentShaderPath(),
                DirectionalShadowUniform::required))
		, m_DirectionalShadowMap(2048)
        , m_ObjectModelPath(
			ResolveObjectModelPath(specification.sceneFilename))
		, m_ObjectSceneAsset(
			Assets::LoadGltfScene(m_ObjectModelPath))
		, m_ObjectLocalBounds(
			CalculateSceneBounds(
				m_ObjectSceneAsset))
		, m_ObjectShaderMonitor(
			ShaderPath("Object.vert"),
			ShaderPath("Object.frag")
		)
		, m_ObjectShaderProgram(
			CreateShaderProgram(
				m_ObjectShaderMonitor.VertexShaderPath(),
				m_ObjectShaderMonitor.FragmentShaderPath(),
				SandboxRendererShaderUniform::required))
		, m_FallbackWhiteTexture(
			1,
			1,
			fallbackWhiteTexturePixels,
			Renderer::OpenGL::TextureColorSpace::Linear)
		, m_FallbackNormalTexture(
			1,
			1,
			fallbackNormalTexturePixels,
			Renderer::OpenGL::TextureColorSpace::Linear)
		, m_DefaultSampler()
		, m_EnvironmentEquirectangularTexture(
			LoadEnvironmentTexture(
				specification.environmentFilename))
		, m_EnvironmentMapProcessor(
			LoadShaderSource("EnvironmentCapture.vert"),
			LoadShaderSource("EquirectangularToCubemap.frag"),
			LoadShaderSource("DiffuseIrradiance.frag"),
			LoadShaderSource("SpecularPrefilter.frag"),
			LoadShaderSource("ToneMapping.vert"),
			LoadShaderSource("BrdfIntegration.frag"))
		, m_EnvironmentCubemap(
			m_EnvironmentMapProcessor
			.ConvertEquirectangularToCubemap(
				m_EnvironmentEquirectangularTexture,
				512))
		, m_SkyboxShaderMonitor(
			ShaderPath("Skybox.vert"),
			ShaderPath("Skybox.frag"))
		, m_SkyboxRenderer(
			LoadTextFile(
				m_SkyboxShaderMonitor.VertexShaderPath()),
			LoadTextFile(
				m_SkyboxShaderMonitor.FragmentShaderPath()))
		, m_DiffuseIrradianceMap(
			m_EnvironmentMapProcessor
			.CreateDiffuseIrradianceMap(
				m_EnvironmentCubemap,
				32))
		, m_PrefilteredSpecularMap(
			m_EnvironmentMapProcessor
			.CreatePrefilteredSpecularMap(
				m_EnvironmentCubemap,
				128,
				5))
		, m_BrdfLookupTexture(
			m_EnvironmentMapProcessor
			.CreateBrdfLookupTexture(
				256))
		, m_GridVertices(
			ForgeSim::Renderer::CreateReferenceGridLines(
				ForgeSim::Renderer::ReferenceGridSpecification{}))
		, m_GridShaderMonitor(
			ShaderPath("Grid.vert"),
			ShaderPath("Grid.frag"))
		, m_DebugLineRenderer(
			LoadTextFile(
				m_GridShaderMonitor.VertexShaderPath()),
			LoadTextFile(
				m_GridShaderMonitor.FragmentShaderPath()))
		, m_ToneMappingShaderMonitor(
			ShaderPath("ToneMapping.vert"),
			ShaderPath("ToneMapping.frag"))
		, m_ToneMappingRenderer(
			LoadTextFile(
				m_ToneMappingShaderMonitor.VertexShaderPath()),
			LoadTextFile(
				m_ToneMappingShaderMonitor.FragmentShaderPath()))
	{

		UploadObjectModelResources(
			m_ObjectSceneAsset,
			m_GpuPrimitiveResources,
			m_GpuTextures,
			m_GpuSamplers,
			m_GpuMaterialResources);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}

	void SandboxRenderer::AppendSceneNodeSubmissions(
		Assets::SceneNodeIndex nodeIndex,
		const glm::mat4& parentNodeTransform,
		const glm::mat4& objectTransform,
		bool isSelected)
	{
		if (nodeIndex >= m_ObjectSceneAsset.nodes.size())
		{
			throw std::runtime_error(
				"Scene node index is invalid.");
		}

		const Assets::SceneNodeData& node =
			m_ObjectSceneAsset.nodes[nodeIndex];

		const glm::mat4 nodeTransform =
			parentNodeTransform *
			CreateSceneNodeLocalMatrix(
				node.localTransform);

		if (node.meshIndex)
		{
			if (*node.meshIndex >=
				m_ObjectSceneAsset.meshes.size())
			{
				throw std::runtime_error(
					"Scene node references an invalid mesh.");
			}

			for (const GpuPrimitiveResource& resource :
				m_GpuPrimitiveResources)
			{
				if (resource.sceneMeshIndex !=
					*node.meshIndex)
				{
					continue;
				}

				m_RenderSubmissions.push_back(
					RenderSubmission{
						.mesh = &resource.mesh,
						.materialIndex =
							resource.materialIndex,
						.modelTransform =
							objectTransform *
							nodeTransform,
						.isSelected = isSelected
					});
			}
		}

		for (const Assets::SceneNodeIndex childIndex :
		node.childIndices)
		{
			AppendSceneNodeSubmissions(
				childIndex,
				nodeTransform,
				objectTransform,
				isSelected);
		}
	}

	void SandboxRenderer::UploadPointLights(
		std::span<const PointLight> lights)
	{
		const PointLightSubmission submission =
			PreparePointLights(lights);

		for (std::size_t index = 0;
			index < submission.count;
			++index)
		{
			const PreparedPointLight& light =
				submission.lights[index];

			const std::string prefix =
				"uPointLights[" +
				std::to_string(index) +
				"].";

			m_ObjectShaderProgram.SetVector3(
				prefix + "position",
				light.position);

			m_ObjectShaderProgram.SetVector3(
				prefix + "radiance",
				light.radiance);

			m_ObjectShaderProgram.SetFloat(
				prefix + "range",
				light.range);
		}

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::pointLightCount,
			static_cast<int>(submission.count));
	}

	void SandboxRenderer::UploadSpotLights(
		std::span<const SpotLight> lights)
	{
		const SpotLightSubmission submission =
			PrepareSpotLights(lights);

		for (std::size_t index = 0;
			index < submission.count;
			++index)
		{
			const PreparedSpotLight& light =
				submission.lights[index];

			const std::string prefix =
				"uSpotLights[" +
				std::to_string(index) +
				"].";

			m_ObjectShaderProgram.SetVector3(
				prefix + "position",
				light.position);

			m_ObjectShaderProgram.SetVector3(
				prefix + "emissionDirection",
				light.emissionDirection);

			m_ObjectShaderProgram.SetVector3(
				prefix + "radiance",
				light.radiance);

			m_ObjectShaderProgram.SetFloat(
				prefix + "range",
				light.range);

			m_ObjectShaderProgram.SetFloat(
				prefix + "innerConeCos",
				light.innerConeCos);

			m_ObjectShaderProgram.SetFloat(
				prefix + "outerConeCos",
				light.outerConeCos);
		}

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::spotLightCount,
			static_cast<int>(submission.count));
	}

	void SandboxRenderer::UploadObjectModelResources(
		const Assets::SceneAsset& sceneAsset,
		std::vector<GpuPrimitiveResource>& primitives,
		std::vector<GpuTextureResource>& textures,
		std::vector<Renderer::OpenGL::Sampler>& samplers,
		std::vector<GpuMaterialResource>& materials)
	{
				// Upload imported mesh primitives once.
		std::size_t primitiveCount = 0;

		for (const Assets::SceneMeshData& mesh :
			sceneAsset.meshes)
		{
			primitiveCount += mesh.primitives.size();
		}

		primitives.reserve(primitiveCount);

		for (Assets::SceneMeshIndex meshIndex = 0;
			meshIndex < sceneAsset.meshes.size();
			++meshIndex)
		{
			const Assets::SceneMeshData& sceneMesh =
				sceneAsset.meshes[meshIndex];

			for (const Assets::MeshPrimitiveData& primitive :
				sceneMesh.primitives)
			{
				primitives.push_back(
					GpuPrimitiveResource{
						.mesh =
							Renderer::OpenGL::GpuMesh{
								primitive.geometry
							},
						.sceneMeshIndex = meshIndex,
						.materialIndex =
							primitive.materialIndex
					});
			}
		}

		if (primitives.empty())
		{
			throw std::runtime_error(
				"Cannot render a scene without mesh primitives.");
		}

		const Assets::TextureResources& textureResources =
			sceneAsset.textureResources;

		// A source texture may need both an sRGB and a linear GPU copy.
		textures.reserve(
			textureResources.textures.size() * 2);

		samplers.reserve(
			textureResources.samplers.size());

		for (const Assets::SamplerData& sampler :
			textureResources.samplers)
		{
			samplers.emplace_back(
				CreateSamplerSpecification(sampler));
		}

		const auto resolveGpuTexture =
			[&textures, &textureResources](
				std::size_t textureIndex,
				Renderer::OpenGL::TextureColorSpace colorSpace)
			-> const Renderer::OpenGL::Texture2D*
			{
				if (textureIndex >=
					textureResources.textures.size())
				{
					throw std::runtime_error(
						"Material references an invalid texture.");
				}

				const auto existingTexture =
					std::find_if(
						textures.begin(),
						textures.end(),
						[textureIndex, colorSpace](
							const GpuTextureResource& resource)
						{
							return
								resource.sourceTextureIndex ==
								textureIndex &&
								resource.colorSpace ==
								colorSpace;

						});

				if (existingTexture != textures.end())
				{
					return &existingTexture->texture;
				}

				const Assets::TextureData& texture =
					textureResources.textures[textureIndex];

				if (texture.imageIndex >=
					textureResources.images.size())
				{
					throw std::runtime_error(
						"Texture references an invalid image index.");
				}

				const Assets::ImageData& image =
					textureResources.images[texture.imageIndex];

				if (!image.IsValid())
				{
					throw std::runtime_error(
						"Cannot upload an invalid texture image.");
				}

				textures.push_back(
					GpuTextureResource{
						.sourceTextureIndex = textureIndex,
						.colorSpace = colorSpace,
						.texture =
							Renderer::OpenGL::Texture2D{
								image.width,
								image.height,
								image.pixels,
								colorSpace
							}
					});

				return &textures.back().texture;
			};


		const auto resolveTextureBinding =
			[this, &samplers, &textureResources, &resolveGpuTexture](
				const std::optional<
				Assets::MaterialTextureReference>& reference,
				const Renderer::OpenGL::Texture2D& fallbackTexture,
				Renderer::OpenGL::TextureColorSpace colorSpace)
			-> GpuTextureBinding
			{
				GpuTextureBinding binding{
					.texture = &fallbackTexture,
					.sampler = &m_DefaultSampler
				};

				if (!reference)
				{
					return binding;
				}

				if (reference->textureCoordinateSet != 0)
				{
					throw std::runtime_error(
						"Only texture-coordinate set zero "
						"is currently supported.");
				}

				if (reference->textureIndex >=
					textureResources.textures.size())
				{
					throw std::runtime_error(
						"Material references an invalid texture.");
				}

				const Assets::TextureData& texture =
					textureResources.textures[
						reference->textureIndex];

				binding.texture =
					resolveGpuTexture(
						reference->textureIndex,
						colorSpace);

				if (texture.samplerIndex)
				{
					if (*texture.samplerIndex >=
						samplers.size())
					{
						throw std::runtime_error(
							"Texture references an invalid sampler.");
					}

					binding.sampler =
						&samplers[
							*texture.samplerIndex];
				}

				return binding;
			};


		materials.reserve(
			sceneAsset.materials.size());

		for (const Assets::MaterialData& material :
			sceneAsset.materials)
		{
			const auto& metallicRoughness =
				material.metallicRoughness;

			const auto& baseColorFactor =
				metallicRoughness.baseColorFactor;

			std::optional<Assets::MaterialTextureReference>
				normalTextureReference;

			float normalScale = 1.0f;

			if (material.normalTexture)
			{
				normalTextureReference =
					material.normalTexture->texture;

				normalScale =
					material.normalTexture->scale;
			}

			std::optional<Assets::MaterialTextureReference>
				occlusionTextureReference;

			float occlusionStrength = 1.0f;

			if (material.occlusionTexture)
			{
				occlusionTextureReference =
					material.occlusionTexture->texture;

				occlusionStrength =
					material.occlusionTexture->strength;
			}

			std::optional<Assets::MaterialTextureReference>
				emissiveTextureReference;

			if (material.emissiveTexture)
			{
				emissiveTextureReference =
					*material.emissiveTexture;
			}

			const glm::vec3 emissiveFactor{
				material.emissiveFactor[0],
				material.emissiveFactor[1],
				material.emissiveFactor[2]
			};

			GpuMaterialResource gpuMaterial{
				.baseColor =
					resolveTextureBinding(
						metallicRoughness.baseColorTexture,
						m_FallbackWhiteTexture,
						Renderer::OpenGL::TextureColorSpace::Srgb),

				.metallicRoughness =
					resolveTextureBinding(
						metallicRoughness.metallicRoughnessTexture,
						m_FallbackWhiteTexture,
						Renderer::OpenGL::TextureColorSpace::Linear),

				.normal =
					resolveTextureBinding(
						normalTextureReference,
						m_FallbackNormalTexture,
						Renderer::OpenGL::TextureColorSpace::Linear),

				.occlusion =
					resolveTextureBinding(
						occlusionTextureReference,
						m_FallbackWhiteTexture,
						Renderer::OpenGL::TextureColorSpace::Linear),

				.emissive =
					resolveTextureBinding(
						emissiveTextureReference,
						m_FallbackWhiteTexture,
						Renderer::OpenGL::TextureColorSpace::Srgb),

				.baseColorFactor = glm::vec4{
					baseColorFactor[0],
					baseColorFactor[1],
					baseColorFactor[2],
					baseColorFactor[3]
				},

				.emissiveFactor = emissiveFactor,

				.metallicFactor =
					metallicRoughness.metallicFactor,

				.roughnessFactor =
					metallicRoughness.roughnessFactor,

				.normalScale = normalScale,

				.occlusionStrength = occlusionStrength,

				.alphaMode = material.alphaMode,

				.alphaCutoff = material.alphaCutoff,

				.doubleSided = material.doubleSided
			};

			materials.push_back(
				gpuMaterial);
		}
	}

	void SandboxRenderer::Render(
		std::span<const SandboxObject> objects,
		std::optional<SandboxObjectId> selectedObjectId,
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::vec3& cameraPosition,
		const SceneLightingView& lighting,
		const SandboxRenderSettings& settings)
	{
		if (settings.framebufferWidth == 0 ||
			settings.framebufferHeight == 0)
		{
			return;
		}

		const DirectionalLight& light =
			lighting.directionalLight;

		m_BoundsVertices.clear();

		if (settings.showObjectBounds)
		{
			m_BoundsVertices.reserve(
				objects.size() * 24);
		}

		m_RenderSubmissions.clear();
		m_RenderSubmissions.reserve(
			objects.size() *
			m_GpuPrimitiveResources.size());

		for (const SandboxObject& object : objects)
		{
			const bool isSelected =
				selectedObjectId &&
				*selectedObjectId == object.id;

			const glm::mat4 objectTransform =
				object.transform.ModelMatrix();

			for (const Assets::SceneNodeIndex rootNodeIndex :
			m_ObjectSceneAsset.rootNodeIndices)
			{
				AppendSceneNodeSubmissions(
					rootNodeIndex,
					glm::mat4{ 1.0f },
					objectTransform,
					isSelected);
			}

			if (settings.showObjectBounds)
			{
				const glm::vec3 boundsColor =
					isSelected
					? glm::vec3{
						1.0f,
						0.85f,
						0.1f
				}
					: glm::vec3{
						0.1f,
						0.9f,
						1.0f
				};

				const auto boundsVertices =
					Renderer::CreateWireframeBoxLines(
						m_ObjectLocalBounds,
						objectTransform,
						boundsColor);

				m_BoundsVertices.insert(
					m_BoundsVertices.end(),
					boundsVertices.begin(),
					boundsVertices.end());
			}
		}


        if (settings.directionalShadows.enabled && light.enabled &&
            !m_RenderSubmissions.empty())
        {
            const auto worldBounds = CalculateSceneBounds(
                objects, m_ObjectLocalBounds);

            m_DirectionalLightViewProjection = Renderer::CreateDirectionalShadowMatrix(
                worldBounds, light.directionToLight);

            RenderDirectionalShadowMap(m_DirectionalLightViewProjection);
        }

		if (!m_HdrRenderTarget)
		{
			m_HdrRenderTarget.emplace(
				settings.framebufferWidth,
				settings.framebufferHeight);
		}
		else
		{
			m_HdrRenderTarget->Resize(
				settings.framebufferWidth,
				settings.framebufferHeight);
		}

		m_HdrRenderTarget->Bind();

		glClearColor(
			settings.backgroundColor.r,
			settings.backgroundColor.g,
			settings.backgroundColor.b,
			1.0f);

		glViewport(
			0,
			0,
			static_cast<GLsizei>(
				settings.framebufferWidth),
			static_cast<GLsizei>(
				settings.framebufferHeight));

		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);

		if (settings.showEnvironmentBackground)
		{
			m_SkyboxRenderer.Render(
				m_EnvironmentCubemap,
				view,
				projection,
				settings.environmentIntensity);
		}

		if (settings.showGrid)
		{
			m_DebugLineRenderer.Render(
				m_GridVertices,
				view,
				projection);
		}

		m_ObjectShaderProgram.Bind();

		constexpr std::uint32_t shadowTextureUnit = 8;

		// Use the depth texture's own sampling settings.
		glBindSampler(shadowTextureUnit, 0);

		m_DirectionalShadowMap.BindDepthTexture(
			shadowTextureUnit);

		m_ObjectShaderProgram.SetInt(
			"uDirectionalShadowMap",
			static_cast<int>(shadowTextureUnit));

		m_ObjectShaderProgram.SetMatrix4x4(
			"uDirectionalLightViewProjection",
			m_DirectionalLightViewProjection);

		const bool shadowsActive =
			settings.directionalShadows.enabled &&
			light.enabled &&
			!m_RenderSubmissions.empty();

		m_ObjectShaderProgram.SetInt(
			"uDirectionalShadowsEnabled", shadowsActive ? 1 : 0);

		m_ObjectShaderProgram.SetInt(
			"uDirectionalShadowFilteringEnabled",
			settings.directionalShadows.filteringEnabled ? 1 : 0);

		m_ObjectShaderProgram.SetFloat(
			"uDirectionalShadowMinimumBias",
			settings.directionalShadows.minimumBias);

		m_ObjectShaderProgram.SetFloat(
			"uDirectionalShadowSlopeBias",
			settings.directionalShadows.slopeBias);

		glBindSampler(
			diffuseIrradianceTextureUnit,
			0);

		m_DiffuseIrradianceMap.Bind(
			diffuseIrradianceTextureUnit);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::
			diffuseIrradianceMap,
			static_cast<int>(
				diffuseIrradianceTextureUnit));

		glBindSampler(
			prefilteredSpecularTextureUnit,
			0);

		m_PrefilteredSpecularMap.Bind(
			prefilteredSpecularTextureUnit);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::
			prefilteredSpecularMap,
			static_cast<int>(
				prefilteredSpecularTextureUnit));

		glBindSampler(
			brdfLookupTextureUnit,
			0);

		m_BrdfLookupTexture.Bind(
			brdfLookupTextureUnit);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::
			brdfLookupTexture,
			static_cast<int>(
				brdfLookupTextureUnit));

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::
			maximumSpecularLod,
			static_cast<float>(
				m_PrefilteredSpecularMap
				.MipLevelCount() -
				1));

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::
			environmentIntensity,
			settings.environmentIntensity);

		m_ObjectShaderProgram.SetMatrix4x4(
			SandboxRendererShaderUniform::view,
			view);

		m_ObjectShaderProgram.SetMatrix4x4(
			SandboxRendererShaderUniform::projection,
			projection);

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::cameraPosition,
			cameraPosition);

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::lightDirection,
			light.directionToLight);

		const glm::vec3 lightRadiance =
			light.enabled
			? light.color * light.intensity
			: glm::vec3(0.0f);

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::lightRadiance,
			lightRadiance);

		UploadPointLights(lighting.pointLights);
		UploadSpotLights(lighting.spotLights);

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::selectionColor,
			settings.selectionHighlight.color);

		const auto materialFor =
			[this](const RenderSubmission& submission)
			-> const GpuMaterialResource*
			{
				if (!submission.materialIndex)
				{
					return nullptr;
				}

				if (*submission.materialIndex >=
					m_GpuMaterialResources.size())
				{
					throw std::runtime_error(
						"Render submission references "
						"an invalid material.");
				}

				return &m_GpuMaterialResources[
					*submission.materialIndex];
			};

		const auto alphaModeFor =
			[&materialFor](
				const RenderSubmission& submission)
			{
				const GpuMaterialResource* material =
					materialFor(submission);

				return material != nullptr
					? material->alphaMode
					: Assets::AlphaMode::Opaque;
			};

		const auto cameraDistanceSquared =
			[&cameraPosition](
				const RenderSubmission& submission)
			{
				// Transparent submissions are sorted using
				// their transformed mesh origins.
				const glm::vec3 submissionPosition{
					submission.modelTransform[3]
				};

				const glm::vec3 offset =
					submissionPosition -
					cameraPosition;

				return glm::dot(offset, offset);
			};

		std::stable_sort(
			m_RenderSubmissions.begin(),
			m_RenderSubmissions.end(),
			[&](const RenderSubmission& left,
				const RenderSubmission& right)
			{
				const bool leftIsBlended =
					alphaModeFor(left) ==
					Assets::AlphaMode::Blend;

				const bool rightIsBlended =
					alphaModeFor(right) ==
					Assets::AlphaMode::Blend;

				if (leftIsBlended != rightIsBlended)
				{
					return !leftIsBlended;
				}

				if (!leftIsBlended)
				{
					// Preserve the original order of opaque
					// and masked submissions.
					return false;
				}

				// Transparent geometry uses back-to-front
				// ordering.
				return cameraDistanceSquared(left) >
					cameraDistanceSquared(right);
			});

		const GLboolean cullFaceWasEnabled =
			glIsEnabled(GL_CULL_FACE);

		GLint previousCullFaceMode = GL_BACK;

		glGetIntegerv(
			GL_CULL_FACE_MODE,
			&previousCullFaceMode);

		const GLboolean blendWasEnabled =
			glIsEnabled(GL_BLEND);

		GLboolean previousDepthWriteMask = GL_TRUE;

		glGetBooleanv(
			GL_DEPTH_WRITEMASK,
			&previousDepthWriteMask);

		GLint previousBlendSourceRgb = GL_ONE;
		GLint previousBlendDestinationRgb = GL_ZERO;
		GLint previousBlendSourceAlpha = GL_ONE;
		GLint previousBlendDestinationAlpha = GL_ZERO;

		glGetIntegerv(
			GL_BLEND_SRC_RGB,
			&previousBlendSourceRgb);

		glGetIntegerv(
			GL_BLEND_DST_RGB,
			&previousBlendDestinationRgb);

		glGetIntegerv(
			GL_BLEND_SRC_ALPHA,
			&previousBlendSourceAlpha);

		glGetIntegerv(
			GL_BLEND_DST_ALPHA,
			&previousBlendDestinationAlpha);

		for (const RenderSubmission& submission :
			m_RenderSubmissions)
		{
			if (submission.mesh == nullptr)
			{
				continue;
			}

			const GpuMaterialResource* material =
				materialFor(submission);

			const Assets::AlphaMode alphaMode =
				material != nullptr
				? material->alphaMode
				: Assets::AlphaMode::Opaque;

			const bool doubleSided =
				material != nullptr &&
				material->doubleSided;

			if (doubleSided)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}

			if (alphaMode == Assets::AlphaMode::Blend)
			{
				glEnable(GL_BLEND);

				glBlendFunc(
					GL_SRC_ALPHA,
					GL_ONE_MINUS_SRC_ALPHA);

				// Transparent surfaces continue testing
				// against depth without writing new depth.
				glDepthMask(GL_FALSE);
			}
			else
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
			}

			BindMaterial(
				submission.materialIndex);

			const float selectionAmount =
				submission.isSelected &&
				settings.selectionHighlight.enabled
				? std::clamp(
					settings.selectionHighlight.strength,
					0.0f,
					1.0f)
				: 0.0f;

			m_ObjectShaderProgram.SetFloat(
				SandboxRendererShaderUniform::
				selectionAmount,
				selectionAmount);

			m_ObjectShaderProgram.SetMatrix4x4(
				SandboxRendererShaderUniform::model,
				submission.modelTransform);

			submission.mesh->Bind();

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(
					submission.mesh->IndexCount()),
				GL_UNSIGNED_INT,
				nullptr);
		}

		glCullFace(
			static_cast<GLenum>(
				previousCullFaceMode));

		if (cullFaceWasEnabled == GL_TRUE)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glDepthMask(
			previousDepthWriteMask);

		glBlendFuncSeparate(
			static_cast<GLenum>(
				previousBlendSourceRgb),
			static_cast<GLenum>(
				previousBlendDestinationRgb),
			static_cast<GLenum>(
				previousBlendSourceAlpha),
			static_cast<GLenum>(
				previousBlendDestinationAlpha));

		if (blendWasEnabled == GL_TRUE)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		m_LightVertices.clear();

		if (settings.showLights)
		{
			const auto debugLightColor =
				[&](SceneLightId id, bool enabled, const glm::vec3& normalColor)
				{
					if (settings.selectedLightId &&
						*settings.selectedLightId == id)
					{
						return glm::vec3{ 1.0f, 0.2f, 0.8f };
					}

					return enabled
						? normalColor
						: glm::vec3{ 0.4f };
				};

			const LightDebugLayout layout =
				CalculateLightDebugLayout(
					settings.lightDebugBounds.value_or(m_ObjectLocalBounds));

			const glm::vec3 arrowColor =
				debugLightColor(
					light.id,
					light.enabled,
					glm::vec3{ 1.0f, 0.85f, 0.15f });

			m_LightVertices =
				Renderer::CreateDirectionalLightLines(
					Renderer::DirectionArrowSpecification{
						.origin = light.position,
						.direction = -light.directionToLight,
						.length = layout.directionalLength,
						.color = arrowColor,
						.showArrowheads = false,
						.markerSize = layout.markerSize
					});

			for (const PointLight& pointLight : lighting.pointLights)
			{
				const glm::vec3 pointColor =
					debugLightColor(
						pointLight.id,
						pointLight.enabled,
						glm::vec3{ 0.2f, 0.8f, 1.0f });

				const auto pointVertices =
					Renderer::CreateWireframeSphereLines(
						Renderer::WireframeSphereSpecification{
							.center = pointLight.position,
							.radius = pointLight.range,
							.segmentCount = 32,
							.markerSize = layout.markerSize,
							.color = pointColor
						});

				m_LightVertices.insert(
					m_LightVertices.end(),
					pointVertices.begin(),
					pointVertices.end());
			}

			for (const SpotLight& spotLight : lighting.spotLights)
			{
				const glm::vec3 spotColor =
					debugLightColor(
						spotLight.id,
						spotLight.enabled,
						glm::vec3{ 1.0f, 0.45f, 0.15f });

				const auto spotVertices =
					Renderer::CreateWireframeSpotLightLines(
						Renderer::WireframeSpotLightSpecification{
							.position = spotLight.position,
							.emissionDirection = spotLight.emissionDirection,
							.range = spotLight.range,
							.innerConeAngleRadians =
								spotLight.innerConeAngleRadians,
							.outerConeAngleRadians =
								spotLight.outerConeAngleRadians,
							.segmentCount = 32,
							.markerSize = layout.markerSize,
							.color = spotColor
						});

				m_LightVertices.insert(
					m_LightVertices.end(),
					spotVertices.begin(),
					spotVertices.end());
			}

			const GLboolean depthTestWasEnabled =
				glIsEnabled(GL_DEPTH_TEST);

			glDisable(GL_DEPTH_TEST);

			m_DebugLineRenderer.Render(
				m_LightVertices,
				view,
				projection);

			if (depthTestWasEnabled == GL_TRUE)
			{
				glEnable(GL_DEPTH_TEST);
			}
		}
		
		if (settings.showObjectBounds &&
			!m_BoundsVertices.empty())
		{
			const GLboolean depthTestWasEnabled =
				glIsEnabled(GL_DEPTH_TEST);

			glDisable(GL_DEPTH_TEST);

			m_DebugLineRenderer.Render(
				m_BoundsVertices,
				view,
				projection);

			if (depthTestWasEnabled == GL_TRUE)
			{
				glEnable(GL_DEPTH_TEST);
			}
		}

		if (settings.renderToTexture)
		{
			if (!m_ColorRenderTarget)
			{
				m_ColorRenderTarget.emplace(
					settings.framebufferWidth,
					settings.framebufferHeight);
			}

			// RenderToTarget also resizes the destination when necessary.
			m_ToneMappingRenderer.RenderToTarget(
				*m_HdrRenderTarget,
				*m_ColorRenderTarget,
				settings.exposure);
		}
		else
		{
			m_ToneMappingRenderer.RenderToDefaultFramebuffer(
				*m_HdrRenderTarget,
				settings.exposure);
		}
	}


    void SandboxRenderer::RenderDirectionalShadowMap(
        const glm::mat4& lightViewProjection)
    {
        const DirectionalShadowPassState savedState;

        m_DirectionalShadowMap.Bind();
        const auto resolution = static_cast<GLsizei>(
            m_DirectionalShadowMap.Resolution());
        glViewport(0, 0, resolution, resolution);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDepthRange(0.0, 1.0);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 1.0f);
        glDisable(GL_DEPTH_CLAMP);

        constexpr GLfloat farDepth = 1.0f;
        glClearBufferfv(GL_DEPTH, 0, &farDepth);

        m_DirectionalShadowShaderProgram.Bind();
        m_DirectionalShadowShaderProgram.SetMatrix4x4(
            DirectionalShadowUniform::lightViewProjection,
            lightViewProjection);
        m_DirectionalShadowShaderProgram.SetInt(
            DirectionalShadowUniform::baseColorTexture,
            static_cast<int>(shadowAlphaTextureUnit));

        for (const RenderSubmission& submission : m_RenderSubmissions)
        {
            if (submission.mesh == nullptr)
            {
                continue;
            }

            const GpuMaterialResource* material = nullptr;
            if (submission.materialIndex)
            {
                if (*submission.materialIndex >= m_GpuMaterialResources.size())
                {
                    throw std::runtime_error(
                        "Shadow submission references an invalid material.");
                }
                material = &m_GpuMaterialResources[*submission.materialIndex];
            }

            // Blended surfaces do not cast opaque shadows in this version.
            if (material != nullptr &&
                material->alphaMode == Assets::AlphaMode::Blend)
            {
                continue;
            }

            if (material != nullptr && material->doubleSided)
            {
                glDisable(GL_CULL_FACE);
            }
            else
            {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }

            const GpuTextureBinding baseColor = material != nullptr
                ? material->baseColor
                : GpuTextureBinding{
                    .texture = &m_FallbackWhiteTexture,
                    .sampler = &m_DefaultSampler
                };

            baseColor.texture->Bind(shadowAlphaTextureUnit);
            baseColor.sampler->Bind(shadowAlphaTextureUnit);

            m_DirectionalShadowShaderProgram.SetFloat(
                DirectionalShadowUniform::baseColorAlpha,
                material != nullptr ? material->baseColorFactor.a : 1.0f);
            m_DirectionalShadowShaderProgram.SetInt(
                DirectionalShadowUniform::alphaMaskEnabled,
                material != nullptr &&
                material->alphaMode == Assets::AlphaMode::Mask ? 1 : 0);
            m_DirectionalShadowShaderProgram.SetFloat(
                DirectionalShadowUniform::alphaCutoff,
                material != nullptr ? material->alphaCutoff : 0.5f);
            m_DirectionalShadowShaderProgram.SetMatrix4x4(
                DirectionalShadowUniform::model,
                submission.modelTransform);

            submission.mesh->Bind();
            glDrawElements(GL_TRIANGLES,
                static_cast<GLsizei>(submission.mesh->IndexCount()),
                GL_UNSIGNED_INT, nullptr);
        }

        // Texture unit 8 is reserved for shadow rendering. The next pass
        // must bind its own texture and sampler before using that unit.
        glBindSampler(shadowAlphaTextureUnit, 0);
        glBindTextureUnit(shadowAlphaTextureUnit, 0);
    }

	unsigned int SandboxRenderer::SceneColorTexture() const noexcept
	{
		return m_ColorRenderTarget
			? m_ColorRenderTarget->ColorTextureHandle()
			: 0;
	}

	void SandboxRenderer::ReloadChangedShaders()
	{
        if (m_DirectionalShadowShaderMonitor.PollForChanges())
        {
            try
            {
                auto replacement = CreateShaderProgram(
                    m_DirectionalShadowShaderMonitor.VertexShaderPath(),
                    m_DirectionalShadowShaderMonitor.FragmentShaderPath(),
                    DirectionalShadowUniform::required);

                m_DirectionalShadowShaderProgram = std::move(replacement);
                ForgeSim::Core::LogInfo("Reloaded directional-shadow shader program.");
            }
            catch (const std::exception& exception)
            {
                ForgeSim::Core::LogError(
                    "Failed to reload directional-shadow shader program: " +
                    std::string{ exception.what() });
            }
        }

		if (m_ObjectShaderMonitor.PollForChanges())
		{
			try
			{
				auto replacement =
					CreateShaderProgram(
						m_ObjectShaderMonitor.VertexShaderPath(),
						m_ObjectShaderMonitor.FragmentShaderPath(),
						SandboxRendererShaderUniform::required);

				m_ObjectShaderProgram =
					std::move(replacement);

				ForgeSim::Core::LogInfo(
					"Reloaded object shader program.");
			}
			catch (const std::exception& exception)
			{
				ForgeSim::Core::LogError(
					"Failed to reload object shader program: " +
					std::string{ exception.what() });
			}
		}

		if (m_SkyboxShaderMonitor.PollForChanges())
		{
			try
			{
				const std::string vertexShaderSource =
					LoadTextFile(
						m_SkyboxShaderMonitor
						.VertexShaderPath());

				const std::string fragmentShaderSource =
					LoadTextFile(
						m_SkyboxShaderMonitor
						.FragmentShaderPath());

				m_SkyboxRenderer.ReloadShaders(
					vertexShaderSource,
					fragmentShaderSource);

				ForgeSim::Core::LogInfo(
					"Reloaded skybox shader program.");
			}
			catch (const std::exception& exception)
			{
				ForgeSim::Core::LogError(
					"Failed to reload skybox shader program: " +
					std::string{ exception.what() });
			}
		}

		if (m_GridShaderMonitor.PollForChanges())
		{
			try
			{
				const std::string vertexShaderSource =
					LoadTextFile(
						m_GridShaderMonitor.VertexShaderPath());

				const std::string fragmentShaderSource =
					LoadTextFile(
						m_GridShaderMonitor.FragmentShaderPath());

				m_DebugLineRenderer.ReloadShaders(
					vertexShaderSource,
					fragmentShaderSource);

				ForgeSim::Core::LogInfo(
					"Reloaded grid shader program.");
			}
			catch (const std::exception& exception)
			{
				ForgeSim::Core::LogError(
					"Failed to reload grid shader program: " +
					std::string{ exception.what() });
			}
		}

		if (m_ToneMappingShaderMonitor.PollForChanges())
		{
			try
			{
				const std::string vertexShaderSource =
					LoadTextFile(
						m_ToneMappingShaderMonitor
						.VertexShaderPath());

				const std::string fragmentShaderSource =
					LoadTextFile(
						m_ToneMappingShaderMonitor
						.FragmentShaderPath());

				m_ToneMappingRenderer.ReloadShaders(
					vertexShaderSource,
					fragmentShaderSource);

				ForgeSim::Core::LogInfo(
					"Reloaded tone-mapping shader program.");
			}
			catch (const std::exception& exception)
			{
				ForgeSim::Core::LogError(
					"Failed to reload tone-mapping shader program: " +
					std::string{ exception.what() });
			}
		}
	}

	void SandboxRenderer::LoadObjectModel(
		const std::filesystem::path& path)
	{
		if (path.empty() || path.filename().empty())
		{
			throw std::invalid_argument(
				"Model path must name a file.");
		}

		std::filesystem::path replacementPath =
			std::filesystem::weakly_canonical(
				std::filesystem::absolute(path));

		Assets::SceneAsset replacementScene =
			Assets::LoadGltfScene(replacementPath);

		const Renderer::AxisAlignedBoundingBox replacementBounds =
			CalculateSceneBounds(replacementScene);

		std::vector<GpuPrimitiveResource> replacementPrimitives;
		std::vector<GpuTextureResource> replacementTextures;
		std::vector<Renderer::OpenGL::Sampler> replacementSamplers;
		std::vector<GpuMaterialResource> replacementMaterials;

		UploadObjectModelResources(
			replacementScene,
			replacementPrimitives,
			replacementTextures,
			replacementSamplers,
			replacementMaterials);

		// Everything that can fail during import/upload has completed.
		static_assert(
			std::is_nothrow_swappable_v<Assets::SceneAsset>);

		using std::swap;

		m_ObjectModelPath.swap(replacementPath);
		swap(m_ObjectSceneAsset, replacementScene);
		m_ObjectLocalBounds = replacementBounds;

		m_GpuPrimitiveResources.swap(replacementPrimitives);
		m_GpuTextures.swap(replacementTextures);
		m_GpuSamplers.swap(replacementSamplers);
		m_GpuMaterialResources.swap(replacementMaterials);

		// Previous submissions contain pointers into the old resources.
		m_RenderSubmissions.clear();
		m_BoundsVertices.clear();
	}


	const Renderer::AxisAlignedBoundingBox&
		SandboxRenderer::ObjectLocalBounds() const noexcept
	{
		return m_ObjectLocalBounds;
	}

	const std::filesystem::path&
		SandboxRenderer::ObjectModelPath() const noexcept
	{
		return m_ObjectModelPath;
	}

	void SandboxRenderer::PrepareSceneTexture(
		std::uint32_t width,
		std::uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			return;
		}

		if (!m_ColorRenderTarget)
		{
			m_ColorRenderTarget.emplace(width, height);
		}
		else
		{
			m_ColorRenderTarget->Resize(width, height);
		}
	}

	void SandboxRenderer::ClearWindow(
		std::uint32_t width,
		std::uint32_t height)
	{
		Renderer::OpenGL::HdrRenderTarget::BindDefault();

		glViewport(
			0,
			0,
			static_cast<GLsizei>(width),
			static_cast<GLsizei>(height));

		glClearColor(0.025f, 0.025f, 0.025f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void SandboxRenderer::BindMaterial(
		std::optional<Assets::MaterialIndex> materialIndex)
	{
		GpuTextureBinding baseColor{
			.texture = &m_FallbackWhiteTexture,
			.sampler = &m_DefaultSampler
		};

		GpuTextureBinding metallicRoughness{
			.texture = &m_FallbackWhiteTexture,
			.sampler = &m_DefaultSampler
		};

		GpuTextureBinding normal{
			.texture = &m_FallbackNormalTexture,
			.sampler = &m_DefaultSampler
		};

		GpuTextureBinding occlusion{
			.texture = &m_FallbackWhiteTexture,
			.sampler = &m_DefaultSampler
		};

		GpuTextureBinding emissive{
			.texture = &m_FallbackWhiteTexture,
			.sampler = &m_DefaultSampler
		};

		float normalScale = 1.0f;

		glm::vec4 baseColorFactor{ 1.0f };
		
		Assets::AlphaMode alphaMode = 
			Assets::AlphaMode::Opaque;

		float alphaCutoff = 0.5f;
		bool doubleSided = false;

		glm::vec3 emissiveFactor{ 0.0f };

		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		float occlusionStrength = 1.0f;

		if (materialIndex)
		{
			if (*materialIndex >=
				m_GpuMaterialResources.size())
			{
				throw std::runtime_error(
					"Render submission references "
					"an invalid material.");
			}

			const GpuMaterialResource& material =
				m_GpuMaterialResources[*materialIndex];

			baseColor = material.baseColor;

			alphaMode = material.alphaMode;
			alphaCutoff = material.alphaCutoff;

			doubleSided = material.doubleSided;

			metallicRoughness =
				material.metallicRoughness;

			normal = material.normal;

			normalScale = material.normalScale;

			baseColorFactor =
				material.baseColorFactor;

			metallicFactor =
				material.metallicFactor;

			roughnessFactor =
				material.roughnessFactor;

			occlusion = material.occlusion;

			occlusionStrength = material.occlusionStrength;

			emissive = material.emissive;
			emissiveFactor = material.emissiveFactor;
		}

		baseColor.texture->Bind(
			baseColorTextureUnit);

		baseColor.sampler->Bind(
			baseColorTextureUnit);

		metallicRoughness.texture->Bind(
			metallicRoughnessTextureUnit);

		metallicRoughness.sampler->Bind(
			metallicRoughnessTextureUnit);

		normal.texture->Bind(normalTextureUnit);
		normal.sampler->Bind(normalTextureUnit);

		occlusion.texture->Bind(
			occlusionTextureUnit);

		occlusion.sampler->Bind(
			occlusionTextureUnit);

		emissive.texture->Bind(
			emissiveTextureUnit);

		emissive.sampler->Bind(
			emissiveTextureUnit);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::normalTexture,
			static_cast<int>(normalTextureUnit));

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::normalScale,
			normalScale);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::baseColorTexture,
			static_cast<int>(baseColorTextureUnit));

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::alphaMaskEnabled,
			alphaMode == Assets::AlphaMode::Mask
			? 1
			: 0);

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::alphaCutoff,
			alphaCutoff);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::doubleSided,
			doubleSided ? 1 : 0);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::metallicRoughnessTexture,
			static_cast<int>(
				metallicRoughnessTextureUnit));

		m_ObjectShaderProgram.SetVector4(
			SandboxRendererShaderUniform::baseColorFactor,
			baseColorFactor);

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::metallicFactor,
			metallicFactor);

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::roughnessFactor,
			roughnessFactor);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::occlusionTexture,
			static_cast<int>(occlusionTextureUnit));

		m_ObjectShaderProgram.SetFloat(
			SandboxRendererShaderUniform::occlusionStrength,
			occlusionStrength);

		m_ObjectShaderProgram.SetInt(
			SandboxRendererShaderUniform::emissiveTexture,
			static_cast<int>(
				emissiveTextureUnit));

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::emissiveFactor,
			emissiveFactor);
	}
}