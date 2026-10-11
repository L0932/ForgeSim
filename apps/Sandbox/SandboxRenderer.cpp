#include "SandboxRenderer.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glad/gl.h>

#include <ForgeSim/Assets/GltfLoader.hpp>

#ifndef FORGESIM_SANDBOX_ASSET_DIRECTORY
#error FORGESIM_SANDBOX_ASSET_DIRECTORY must be defined by CMake
#endif

namespace ForgeSim::Sandbox
{
	namespace
	{
		struct GridVertex
		{
			float position[3];
			float color[3];
		};

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

		[[nodiscard]]
		ForgeSim::Renderer::OpenGL::ShaderProgram CreateShaderProgram(
			const std::filesystem::path& vertexShaderFilename,
			const std::filesystem::path& fragmentShaderFilename)
		{
			const std::filesystem::path shaderDirectory =
				std::filesystem::path{
					FORGESIM_SANDBOX_ASSET_DIRECTORY
			} / "shaders";

			const std::filesystem::path vertexShaderPath =
				shaderDirectory / vertexShaderFilename;

			const std::filesystem::path fragmentShaderPath =
				shaderDirectory / fragmentShaderFilename;

			const std::string vertexShaderSource =
				LoadTextFile(vertexShaderPath);

			const std::string fragmentShaderSource =
				LoadTextFile(fragmentShaderPath);

			return ForgeSim::Renderer::OpenGL::ShaderProgram{
				vertexShaderSource,
				fragmentShaderSource
			};
		}

		[[nodiscard]] std::vector<GridVertex> CreateGridVertices()
		{
			constexpr int halfLineCount = 10;
			constexpr float spacing = 1.0f;
			constexpr float gridHeight = -0.75f;

			constexpr float gridColor = 0.35f;

			const float extent =
				static_cast<float>(halfLineCount) * spacing;

			std::vector<GridVertex> gridVertices;

			gridVertices.reserve(
				static_cast<std::size_t>(
					(halfLineCount * 2 + 1) * 4));

			for (int line = -halfLineCount; line <= halfLineCount; ++line)
			{
				const float offset =
					static_cast<float>(line) * spacing;

				// Lines parallel to X. The center line represents the X axis.
				const float xAxisRed =
					line == 0 ? 0.9f : gridColor;

				const float xAxisGreen =
					line == 0 ? 0.2f : gridColor;

				const float xAxisBlue =
					line == 0 ? 0.2f : gridColor;

				gridVertices.push_back(
					GridVertex{
						.position = {
							-extent,
							gridHeight,
							offset
						},
						.color = {
							xAxisRed,
							xAxisGreen,
							xAxisBlue
						}
					});

				gridVertices.push_back(
					GridVertex{
						.position = {
							extent,
							gridHeight,
							offset
						},
						.color = {
							xAxisRed,
							xAxisGreen,
							xAxisBlue
						}
					});

				// Lines parallel to Z. The center line represents the Z axis.
				const float zAxisRed =
					line == 0 ? 0.2f : gridColor;

				const float zAxisGreen =
					line == 0 ? 0.4f : gridColor;

				const float zAxisBlue =
					line == 0 ? 0.9f : gridColor;

				gridVertices.push_back(
					GridVertex{
						.position = {
							offset,
							gridHeight,
							-extent
						},
						.color = {
							zAxisRed,
							zAxisGreen,
							zAxisBlue
						}
					});

				gridVertices.push_back(
					GridVertex{
						.position = {
							offset,
							gridHeight,
							extent
						},
						.color = {
							zAxisRed,
							zAxisGreen,
							zAxisBlue
						}
					});
			}
			return gridVertices;
		}

		const std::vector<GridVertex> gridVertices =
			CreateGridVertices();

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
	}

	SandboxRenderer::SandboxRenderer()
		: m_ObjectMesh(
			LoadObjectMesh("BoxVertexColors.glb"))
		, m_ObjectShaderProgram(
			CreateShaderProgram(
				"Object.vert",
				"Object.frag"))
		, m_GridShaderProgram(
			CreateShaderProgram(
				"Grid.vert",
				"Grid.frag"))
		, m_VertexBuffer(
			m_ObjectMesh.vertices.data(),
			m_ObjectMesh.vertices.size() *
			sizeof(Assets::MeshVertex))
		, m_IndexBuffer(
			m_ObjectMesh.indices.data(),
			m_ObjectMesh.indices.size() *
			sizeof(std::uint32_t))
		, m_VertexArray()
		, m_GridVertexBuffer(
			gridVertices.data(),
			gridVertices.size() *
			sizeof(GridVertex))
		, m_GridVertexArray()
		, m_GridVertexCount(gridVertices.size())
		// Vertex-array configuration and OpenGL state initialization.
	{
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

		m_GridVertexArray.SetVertexBuffer(
			m_GridVertexBuffer,
			vertexBindingIndex,
			sizeof(GridVertex));

		m_GridVertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(GridVertex, position));

		m_GridVertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(GridVertex, color));

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
		const glm::mat4& projection)
	{
		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);

		// Render the reference grid.
		m_GridShaderProgram.Bind();

		m_GridShaderProgram.SetMatrix4x4(
			"uModel",
			glm::mat4{ 1.0f });

		m_GridShaderProgram.SetMatrix4x4(
			"uView",
			view);

		m_GridShaderProgram.SetMatrix4x4(
			"uProjection",
			projection);

		m_GridVertexArray.Bind();

		glDrawArrays(
			GL_LINES,
			0,
			static_cast<GLsizei>(
				m_GridVertexCount));

		// Render Sandbox objects.
		m_ObjectShaderProgram.Bind();

		m_ObjectShaderProgram.SetMatrix4x4(
			"uView",
			view);

		m_ObjectShaderProgram.SetMatrix4x4(
			"uProjection",
			projection);

		m_VertexArray.Bind();

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
		}
	}
}