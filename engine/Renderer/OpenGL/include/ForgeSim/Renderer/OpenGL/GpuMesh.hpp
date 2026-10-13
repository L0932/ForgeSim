#pragma once

#include <cstddef>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class GpuMesh final
	{
	public:
		explicit GpuMesh(
			const Assets::MeshData& mesh);

		GpuMesh(const GpuMesh&) = delete;
		GpuMesh& operator=(const GpuMesh&) = delete;

		GpuMesh(GpuMesh&&) noexcept = default;
		GpuMesh& operator=(GpuMesh&&) noexcept = default;

		void Bind() const noexcept;

		[[nodiscard]] std::size_t IndexCount()
			const noexcept;

	private:
		Buffer m_VertexBuffer;
		Buffer m_IndexBuffer;
		VertexArray m_VertexArray;

		std::size_t m_IndexCount = 0;
	};
}