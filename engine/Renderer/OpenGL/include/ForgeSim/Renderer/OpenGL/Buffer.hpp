#pragma once

#include <cstddef>

namespace ForgeSim::Renderer::OpenGL
{
	class Buffer final
	{
	public:
		Buffer(
			const void* data,
			std::size_t sizeInBytes
		);

		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		Buffer(Buffer&&) = delete;
		Buffer& operator=(Buffer&&) = delete;

		[[nodiscard]] unsigned int Handle() const noexcept;

	private:
		unsigned int m_Handle = 0;
	};
}