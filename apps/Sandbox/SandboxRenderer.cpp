#include "SandboxRenderer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ForgeSim/Assets/GltfLoader.hpp>
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
		constexpr std::array<std::uint8_t, 4>
			fallbackWhiteTexturePixels{
				255,
				255,
				255,
				255
		};

		constexpr std::uint32_t baseColorTextureUnit = 0;

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
			const std::filesystem::path& fragmentShaderFilename)
		{
			const std::string vertexShaderSource =
				LoadShaderSource(vertexShaderFilename);

			const std::string fragmentShaderSource =
				LoadShaderSource(fragmentShaderFilename);

			return ForgeSim::Renderer::OpenGL::ShaderProgram{
				vertexShaderSource,
				fragmentShaderSource
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
	}

	SandboxRenderer::SandboxRenderer()
		: m_ObjectSceneAsset(
			LoadObjectScene("BoxTextured.glb"))
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
				m_ObjectShaderMonitor.FragmentShaderPath()))
		, m_FallbackWhiteTexture(
			1,
			1,
			fallbackWhiteTexturePixels)
		, m_DefaultSampler()
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

		m_GpuTextures.reserve(
			textureResources.textures.size());

		for (const Assets::TextureData& texture :
			textureResources.textures)
		{
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

			m_GpuTextures.emplace_back(
				image.width,
				image.height,
				image.pixels);
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
		bool showGrid,
		bool showObjectBounds)
	{
		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);

		if (showGrid)
		{
			// Render the reference grid.
			m_DebugLineRenderer.Render(
				m_GridVertices,
				view,
				projection);
		}

		// Render Sandbox objects.
		m_ObjectShaderProgram.Bind();

		m_ObjectShaderProgram.SetMatrix4x4(
			"uView",
			view);

		m_ObjectShaderProgram.SetMatrix4x4(
			"uProjection",
			projection);

		m_BoundsVertices.clear();

		if (showObjectBounds)
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

			if (showObjectBounds)
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

			m_ObjectShaderProgram.SetFloat(
				"uSelectionAmount",
				submission.isSelected ? 0.55f : 0.0f);

			m_ObjectShaderProgram.SetMatrix4x4(
				"uModel",
				submission.modelTransform);

			submission.mesh->Bind();

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(
					submission.mesh->IndexCount()),
				GL_UNSIGNED_INT,
				nullptr);
		}

		if (showObjectBounds && !m_BoundsVertices.empty())
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
						m_ObjectShaderMonitor.FragmentShaderPath());

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
	}


	const Renderer::AxisAlignedBoundingBox&
		SandboxRenderer::ObjectLocalBounds() const noexcept
	{
		return m_ObjectLocalBounds;
	}

	void SandboxRenderer::BindMaterial(
		std::optional<Assets::MaterialIndex> materialIndex)
	{
		const Renderer::OpenGL::Texture2D* baseColorTexture =
			&m_FallbackWhiteTexture;

		glm::vec4 baseColorFactor{ 1.0f };

		if (materialIndex)
		{
			if (*materialIndex >=
				m_ObjectSceneAsset.materials.size())
			{
				throw std::runtime_error(
					"Render submission references an invalid material.");
			}

			const Assets::MaterialData& material =
				m_ObjectSceneAsset.materials[*materialIndex];

			const auto& factor =
				material.metallicRoughness.baseColorFactor;

			baseColorFactor = glm::vec4{
				factor[0],
				factor[1],
				factor[2],
				factor[3]
			};

			const auto& textureReference =
				material.metallicRoughness.baseColorTexture;

			if (textureReference)
			{
				if (textureReference->textureIndex >=
					m_GpuTextures.size())
				{
					throw std::runtime_error(
						"Material references an invalid GPU texture.");
				}

				baseColorTexture =
					&m_GpuTextures[
						textureReference->textureIndex];
			}
		}

		baseColorTexture->Bind(baseColorTextureUnit);
		m_DefaultSampler.Bind(baseColorTextureUnit);

		m_ObjectShaderProgram.SetInt(
			"uBaseColorTexture",
			static_cast<int>(baseColorTextureUnit));

		m_ObjectShaderProgram.SetVector4(
			"uBaseColorFactor",
			baseColorFactor);
	}
}