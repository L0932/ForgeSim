#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>

#include <cassert>
#include <stdexcept>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_Handle);

		if (m_Handle == 0)
		{
			throw std::runtime_error(
				"Failed to create OpenGL vertex array"
			);
		}
	}

	VertexArray::~VertexArray()
	{
		if (m_Handle != 0)
		{
			glDeleteVertexArrays(1, &m_Handle);
			m_Handle = 0;
		}
	}

	void VertexArray::SetVertexBuffer(
		const Buffer& buffer,
		std::uint32_t bindingIndex,
		std::size_t stride) noexcept
	{
		assert(stride > 0);

		glVertexArrayVertexBuffer(
			m_Handle,
			bindingIndex,
			buffer.Handle(),
			0,
			static_cast<GLsizei>(stride));
	}

	void VertexArray::SetIndexBuffer(
		const Buffer& buffer) noexcept
	{
		glVertexArrayElementBuffer(
			m_Handle,
			buffer.Handle());
	}

	void VertexArray::SetFloatAttribute(
		std::uint32_t attributeIndex,
		std::uint32_t bindingIndex,
		std::uint32_t componentCount,
		std::size_t relativeOffset) noexcept
	{
		assert(componentCount >= 1);
		assert(componentCount <= 4);

		glEnableVertexArrayAttrib(
			m_Handle,
			attributeIndex
		);

		glVertexArrayAttribFormat(
			m_Handle,
			attributeIndex,
			static_cast<GLint>(componentCount),
			GL_FLOAT,
			GL_FALSE,
			static_cast<GLuint>(relativeOffset));

		glVertexArrayAttribBinding(
			m_Handle,
			attributeIndex,
			bindingIndex);
	}

	void VertexArray::Bind() const noexcept
	{
		glBindVertexArray(m_Handle);
	}
}