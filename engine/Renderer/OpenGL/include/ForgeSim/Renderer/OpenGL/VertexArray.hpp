#pragma once

#include <cstddef>
#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
	class Buffer;

	class VertexArray final
	{
	public:
		VertexArray();
		~VertexArray();

		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;

		VertexArray(VertexArray&&) = delete;
		VertexArray& operator=(VertexArray&&) = delete;

		void SetVertexBuffer(
			const Buffer& buffer,
			std::uint32_t bindingIndex,
			std::size_t stride
		) noexcept;

		void SetIndexBuffer(
			const Buffer& buffer) noexcept;

			void SetFloatAttribute(
				std::uint32_t attributeIndex,
				std::uint32_t bindingIndex,
				std::uint32_t componentCount,
				std::size_t relativeOffset
			) noexcept;

		void Bind() const noexcept;

	private:
		unsigned int m_Handle = 0;
	};
}