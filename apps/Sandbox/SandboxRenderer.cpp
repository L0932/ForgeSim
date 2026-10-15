#include "SandboxRenderer.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/ImageLoader.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <ForgeSim/Core/Log.hpp>

#ifndef FORGESIM_SANDBOX_SHADER_DIRECTORY
#error FORGESIM_SANDBOX_SHADER_DIRECTORY must be defined by CMake
#endif

#ifndef FORGESIM_SANDBOX_ASSET_DIRECTORY
#error FORGESIM_SANDBOX_ASSET_DIRECTORY must be defined by CMake
#endif

namespace ForgeSim::Sandbox
{
	namespace
	{
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

			constexpr std::array required{
				model,
				view,
				projection,
				baseColorFactor,
				baseColorTexture,
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
				maximumSpecularLod
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
			return std::filesystem::path{
				FORGESIM_SANDBOX_SHADER_DIRECTORY
			} / filename;
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

		[[nodiscard]] ForgeSim::Assets::SceneAsset LoadObjectScene(
			const std::filesystem::path& filename)
		{
			const std::filesystem::path modelPath =
				std::filesystem::path{
					FORGESIM_SANDBOX_ASSET_DIRECTORY
			} /
				"models" /
				filename;

			return ForgeSim::Assets::LoadGltfScene(modelPath);
		}

		[[nodiscard]]
		Renderer::AxisAlignedBoundingBox CalculateMeshBounds(
			const Assets::MeshData& mesh)
		{
			if (mesh.vertices.empty())
			{
				throw std::runtime_error(
					"Cannot calculate bounds for an empty mesh.");
			}

			const Assets::MeshVertex& firstVertex =
				mesh.vertices.front();

			glm::vec3 minimum{
				firstVertex.position[0],
				firstVertex.position[1],
				firstVertex.position[2]
			};

			glm::vec3 maximum = minimum;

			for (const Assets::MeshVertex& vertex : mesh.vertices)
			{
				const glm::vec3 position{
					vertex.position[0],
					vertex.position[1],
					vertex.position[2]
				};

				minimum = glm::min(minimum, position);
				maximum = glm::max(maximum, position);
			}

			return Renderer::AxisAlignedBoundingBox{
				.minimum = minimum,
				.maximum = maximum
			};
		}

		[[nodiscard]] const Assets::MeshPrimitiveData&
			FirstPrimitive(
				const Assets::SceneAsset& scene)
		{
			if (scene.meshes.empty())
			{
				throw std::runtime_error(
					"Cannot render a scene without meshes.");
			}

			const Assets::SceneMeshData& mesh =
				scene.meshes.front();

			if (mesh.primitives.empty())
			{
				throw std::runtime_error(
					"Cannot render a mesh without primitives.");
			}

			return mesh.primitives.front();
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
			return std::filesystem::path{
				FORGESIM_SANDBOX_ASSET_DIRECTORY
			} /
				"environments" /
				filename;
		}

		[[nodiscard]]
		Renderer::OpenGL::Texture2D LoadEnvironmentTexture(
			const std::filesystem::path& filename)
		{
			const Assets::HdrImageData image =
				Assets::LoadHdrImage(
					EnvironmentPath(filename));

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
	}

	SandboxRenderer::SandboxRenderer()
		: m_ObjectSceneAsset(
			LoadObjectScene("Avocado.glb"))
		, m_ObjectLocalBounds(
			CalculateMeshBounds(
				FirstPrimitive(
					m_ObjectSceneAsset).geometry))
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
				"Studio_Small_01_4k.hdr"))
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
		// Upload imported mesh primitives once.
		std::size_t primitiveCount = 0;

		for (const Assets::SceneMeshData& mesh :
			m_ObjectSceneAsset.meshes)
		{
			primitiveCount += mesh.primitives.size();
		}

		m_GpuPrimitiveResources.reserve(primitiveCount);

		for (Assets::SceneMeshIndex meshIndex = 0;
			meshIndex < m_ObjectSceneAsset.meshes.size();
			++meshIndex)
		{
			const Assets::SceneMeshData& sceneMesh =
				m_ObjectSceneAsset.meshes[meshIndex];

			for (const Assets::MeshPrimitiveData& primitive :
				sceneMesh.primitives)
			{
				m_GpuPrimitiveResources.push_back(
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

		if (m_GpuPrimitiveResources.empty())
		{
			throw std::runtime_error(
				"Cannot render a scene without mesh primitives.");
		}

		const Assets::TextureResources& textureResources =
			m_ObjectSceneAsset.textureResources;

		// A source texture may need both an sRGB and a linear GPU copy.
		m_GpuTextures.reserve(
			textureResources.textures.size() * 2);

		m_GpuSamplers.reserve(
			textureResources.samplers.size());

		for (const Assets::SamplerData& sampler :
			textureResources.samplers)
		{
			m_GpuSamplers.emplace_back(
				CreateSamplerSpecification(sampler));
		}

		const auto resolveGpuTexture =
			[this, &textureResources](
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
						m_GpuTextures.begin(),
						m_GpuTextures.end(),
						[textureIndex, colorSpace](
							const GpuTextureResource& resource)
						{
							return
								resource.sourceTextureIndex ==
								textureIndex &&
								resource.colorSpace ==
								colorSpace;

						});

				if (existingTexture != m_GpuTextures.end())
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

				m_GpuTextures.push_back(
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

				return &m_GpuTextures.back().texture;					
			};


		const auto resolveTextureBinding =
			[this, &textureResources, &resolveGpuTexture](
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
						m_GpuSamplers.size())
					{
						throw std::runtime_error(
							"Texture references an invalid sampler.");
					}

					binding.sampler =
						&m_GpuSamplers[
							*texture.samplerIndex];
				}

				return binding;
			};


		m_GpuMaterialResources.reserve(
			m_ObjectSceneAsset.materials.size());

		for (const Assets::MaterialData& material :
			m_ObjectSceneAsset.materials)
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

				.occlusionStrength = occlusionStrength
			};

			m_GpuMaterialResources.push_back(
				gpuMaterial);
		}
		
		// Configure persistent OpenGL state.
		glClearColor(
			0.05f,
			0.15f,
			0.30f,
			1.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}

	void SandboxRenderer::Render(
		std::span<const SandboxObject> objects,
		std::optional<SandboxObjectId> selectedObjectId,
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::vec3& cameraPosition,
		const DirectionalLight& light,
		const SandboxRenderSettings& settings)
	{
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

		glViewport(
			0,
			0,
			static_cast<GLsizei>(settings.framebufferWidth),
			static_cast<GLsizei>(settings.framebufferHeight));

		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);

		if (settings.showGrid)
		{
			// Render the reference grid.
			m_DebugLineRenderer.Render(
				m_GridVertices,
				view,
				projection);
		}

		// Render Sandbox objects.
		m_ObjectShaderProgram.Bind();

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

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::lightRadiance,
			light.color * light.intensity);

		m_ObjectShaderProgram.SetVector3(
			SandboxRendererShaderUniform::selectionColor,
			settings.selectionHighlight.color);

		m_BoundsVertices.clear();

		if (settings.showObjectBounds)
		{
			m_BoundsVertices.reserve(objects.size() * 24);
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

			for (const GpuPrimitiveResource& resource :
				m_GpuPrimitiveResources)
			{
				// Sandbox objects currently represent mesh zero.
				// Scene-node rendering will provide the mesh index later.
				if (resource.sceneMeshIndex != 0)
				{
					continue;
				}

				m_RenderSubmissions.push_back(
					RenderSubmission{
						.mesh = &resource.mesh,
						.materialIndex =
							resource.materialIndex,
						.modelTransform =
							object.transform.ModelMatrix(),
						.isSelected = isSelected
					});
			}

			if (settings.showObjectBounds)
			{
				const glm::vec3 boundsColor =
					isSelected
					? glm::vec3{ 1.0f, 0.85f, 0.1f }
				: glm::vec3{ 0.1f, 0.9f, 1.0f };

				const auto boundsVertices =
					Renderer::CreateWireframeBoxLines(
						m_ObjectLocalBounds,
						object.transform.ModelMatrix(),
						boundsColor);

				m_BoundsVertices.insert(
					m_BoundsVertices.end(),
					boundsVertices.begin(),
					boundsVertices.end());
			}
		}

		for (const RenderSubmission& submission :
			m_RenderSubmissions)
		{
			if (submission.mesh == nullptr)
			{
				continue;
			}

			BindMaterial(submission.materialIndex);

			const float selectionAmount =
				submission.isSelected &&
				settings.selectionHighlight.enabled
				? std::clamp(
					settings.selectionHighlight.strength,
					0.0f,
					1.0f)
				: 0.0f;

			m_ObjectShaderProgram.SetFloat(
				SandboxRendererShaderUniform::selectionAmount,
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

		if (settings.showObjectBounds && !m_BoundsVertices.empty())
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

		m_ToneMappingRenderer.RenderToDefaultFramebuffer(
			*m_HdrRenderTarget,
			settings.exposure);
	}

	void SandboxRenderer::ReloadChangedShaders()
	{
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


	const Renderer::AxisAlignedBoundingBox&
		SandboxRenderer::ObjectLocalBounds() const noexcept
	{
		return m_ObjectLocalBounds;
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

		glm::vec3 emissiveFactor{ 0.0f };

		float normalScale = 1.0f;

		glm::vec4 baseColorFactor{ 1.0f };

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