#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>

#include <cassert>
#include <stdexcept>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{

	Buffer::Buffer(
		const void* data,
		std::size_t sizeInBytes
	)
	{
		assert(data != nullptr);
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
			GL_STATIC_DRAW);
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
}