#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>

#include <cassert>
#include <stdexcept>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		[[nodiscard]] unsigned int ToOpenGLUsage(
			BufferUsage usage)
		{
			switch (usage)
			{
			case BufferUsage::Static:
				return GL_STATIC_DRAW;

			case BufferUsage::Dynamic:
				return GL_DYNAMIC_DRAW;
			}

			throw std::invalid_argument(
				"Unsupported OpenGL buffer usage.");
		}
	}

	Buffer::Buffer(
		const void* data,
		std::size_t sizeInBytes,
		BufferUsage usage)
		: m_Usage(usage)
	{
		assert(sizeInBytes > 0);

		glCreateBuffers(1, &m_Handle);

		if (m_Handle == 0)
		{
			throw std::runtime_error(
				"Failed to create OpenGL buffer");
		}

		glNamedBufferData(
			m_Handle,
			static_cast<GLsizeiptr>(sizeInBytes),
			data,
			ToOpenGLUsage(m_Usage));
	}

	Buffer::~Buffer()
	{
		if (m_Handle != 0)
		{
			glDeleteBuffers(1, &m_Handle);
			m_Handle = 0;
		}
	}

	unsigned int Buffer::Handle() const noexcept
	{
		return m_Handle;
	}

	void Buffer::SetData(
		const void* data,
		std::size_t sizeInBytes)
	{
		assert(data != nullptr);
		assert(sizeInBytes > 0);

		glNamedBufferData(
			m_Handle,
			static_cast<GLsizeiptr>(sizeInBytes),
			data,
			ToOpenGLUsage(m_Usage));
	}
}