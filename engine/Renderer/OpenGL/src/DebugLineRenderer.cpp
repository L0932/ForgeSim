#include <ForgeSim/Renderer/OpenGL/DebugLineRenderer.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		namespace DebugLineShaderUniform
		{
			constexpr std::string_view view = "uView";
			constexpr std::string_view projection = "uProjection";

			constexpr std::array required {
				view,
				projection
			};
		}
	}

	DebugLineRenderer::DebugLineRenderer(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
		: m_ShaderProgram(
			vertexShaderSource,
			fragmentShaderSource,
			DebugLineShaderUniform::required)
		, m_VertexBuffer(
			nullptr,
			sizeof(DebugLineVertex),
			BufferUsage::Dynamic)
		, m_VertexArray()
	{
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_VertexArray.SetVertexBuffer(
			m_VertexBuffer,
			vertexBindingIndex,
			sizeof(DebugLineVertex));

		m_VertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(DebugLineVertex, position));

		m_VertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(DebugLineVertex, color));
	}

	void DebugLineRenderer::Render(
		std::span<const DebugLineVertex> vertices,
		const glm::mat4& view,
		const glm::mat4& projection)
	{
		if (vertices.empty())
		{
			return;
		}

		m_VertexBuffer.SetData(
			vertices.data(),
			vertices.size_bytes());

		m_ShaderProgram.Bind();

		m_ShaderProgram.SetMatrix4x4(
			DebugLineShaderUniform::view,
			view);

		m_ShaderProgram.SetMatrix4x4(
			DebugLineShaderUniform::projection,
			projection);

		m_VertexArray.Bind();

		glDrawArrays(
			GL_LINES,
			0,
			static_cast<GLsizei>(
				vertices.size()));
	}

	void DebugLineRenderer::ReloadShaders(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
	{
		ShaderProgram replacement{
			vertexShaderSource,
			fragmentShaderSource,
			DebugLineShaderUniform::required
		};

		m_ShaderProgram =
			std::move(replacement);
	}
}