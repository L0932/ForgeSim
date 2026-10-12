#pragma once

#include <cstddef>

namespace ForgeSim::Renderer::OpenGL
{
	enum class BufferUsage
	{
		Static,
		Dynamic
	};

	class Buffer final
	{
	public:
		Buffer(
			const void* data,
			std::size_t sizeInBytes,
			BufferUsage usage = BufferUsage::Static
		);

		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		Buffer(Buffer&&) = delete;
		Buffer& operator=(Buffer&&) = delete;

		[[nodiscard]] unsigned int Handle() const noexcept;

		void SetData(
			const void* data,
			std::size_t sizeInBytes
		);

	private:
		unsigned int m_Handle = 0;
		BufferUsage m_Usage;
	};
}