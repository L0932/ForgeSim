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

#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/ImageLoader.hpp>
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

		[[nodiscard]] ForgeSim::Assets::MeshData LoadObjectMesh(
			const std::filesystem::path& filename)
		{
			const std::filesystem::path modelPath =
				std::filesystem::path{
					FORGESIM_SANDBOX_ASSET_DIRECTORY
			} /
				"models" /
				filename;

			return ForgeSim::Assets::LoadGltfMesh(modelPath);
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

		[[nodiscard]] std::filesystem::path TexturePath(
			const std::filesystem::path& filename)
		{
			return
				std::filesystem::path{
					FORGESIM_SANDBOX_ASSET_DIRECTORY
			} /
				"textures" /
				filename;
		}

		[[nodiscard]]
		Renderer::OpenGL::Texture2D LoadTexture2D(
			const std::filesystem::path& path)
		{
			const Assets::ImageData image =
				Assets::LoadImage(path);

			return Renderer::OpenGL::Texture2D{
				image.width,
				image.height,
				image.pixels
			};
		}
	}

	SandboxRenderer::SandboxRenderer()
		: m_ObjectMesh(
			LoadObjectMesh("BoxTextured.glb"))
		, m_ObjectLocalBounds(
			CalculateMeshBounds(m_ObjectMesh))
		, m_ObjectShaderMonitor(
			ShaderPath("Object.vert"),
			ShaderPath("Object.frag")
		)
		, m_ObjectShaderProgram(
			CreateShaderProgram(
				m_ObjectShaderMonitor.VertexShaderPath(),
				m_ObjectShaderMonitor.FragmentShaderPath()))
		, m_VertexBuffer(
			m_ObjectMesh.vertices.data(),
			m_ObjectMesh.vertices.size() *
			sizeof(Assets::MeshVertex))
		, m_IndexBuffer(
			m_ObjectMesh.indices.data(),
			m_ObjectMesh.indices.size() *
			sizeof(std::uint32_t))
		, m_VertexArray()
		, m_FallbackWhiteTexture(
			1,
			1,
			fallbackWhiteTexturePixels)
		, m_DemonstrationTexture(
			LoadTexture2D(
				TexturePath("Checkerboard.png")))
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
		// Configure object vertex data and persistent OpenGL state.
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_VertexArray.SetVertexBuffer(
			m_VertexBuffer,
			vertexBindingIndex,
			sizeof(Assets::MeshVertex));

		m_VertexArray.SetIndexBuffer(m_IndexBuffer);

		m_VertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(
				Assets::MeshVertex,
				position));

		m_VertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(
				Assets::MeshVertex,
				color));

		m_VertexArray.SetFloatAttribute(
			2,
			vertexBindingIndex,
			2,
			offsetof(
				Assets::MeshVertex,
				textureCoordinates));

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

		// Temporary integration path until glTF material-to-texture
		// associations are imported.
		const Renderer::OpenGL::Texture2D& baseColorTexture =
			m_ObjectMesh.availableAttributes.textureCoordinates
			? m_DemonstrationTexture
			: m_FallbackWhiteTexture;

		baseColorTexture.Bind(baseColorTextureUnit);

		m_DefaultSampler.Bind(baseColorTextureUnit);

		m_ObjectShaderProgram.SetInt(
			"uBaseColorTexture",
			static_cast<int>(baseColorTextureUnit));

		m_DefaultSampler.Bind(
			baseColorTextureUnit);

		m_ObjectShaderProgram.SetInt(
			"uBaseColorTexture",
			static_cast<int>(
				baseColorTextureUnit));

		m_VertexArray.Bind();

		m_BoundsVertices.clear();

		if (showObjectBounds)
		{
			m_BoundsVertices.reserve(objects.size() * 24);
		}

		for (const SandboxObject& object : objects)
		{
			const bool isSelected =
				selectedObjectId &&
				*selectedObjectId == object.id;

			m_ObjectShaderProgram.SetFloat(
				"uSelectionAmount",
				isSelected ? 0.55f : 0.0f);

			m_ObjectShaderProgram.SetMatrix4x4(
				"uModel",
				object.transform.ModelMatrix());

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(
					m_ObjectMesh.indices.size()),
				GL_UNSIGNED_INT,
				nullptr);

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
}