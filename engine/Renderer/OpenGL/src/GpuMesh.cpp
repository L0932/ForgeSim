#include <ForgeSim/Renderer/OpenGL/GpuMesh.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		[[nodiscard]] const ForgeSim::Assets::MeshData&
			ValidateMesh(
				const ForgeSim::Assets::MeshData& mesh)
		{
			if (mesh.vertices.empty())
			{
				throw std::invalid_argument(
					"Cannot create a GPU mesh without vertices.");
			}

			if (mesh.indices.empty())
			{
				throw std::invalid_argument(
					"Cannot create a GPU mesh without indices.");
			}

			return mesh;
		}
	}

	GpuMesh::GpuMesh(
		const Assets::MeshData& mesh)
		: m_VertexBuffer(
			ValidateMesh(mesh).vertices.data(),
			mesh.vertices.size() *
			sizeof(Assets::MeshVertex))
		, m_IndexBuffer(
			mesh.indices.data(),
			mesh.indices.size() *
			sizeof(std::uint32_t))
		, m_VertexArray()
		, m_IndexCount(mesh.indices.size())
	{
		if (mesh.vertices.empty())
		{
			throw std::invalid_argument(
				"Cannot create a GPU mesh without vertices.");
		}

		if (mesh.indices.empty())
		{
			throw std::invalid_argument(
				"Cannot create a GPU mesh without indices.");
		}

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

		m_VertexArray.SetFloatAttribute(
			3,
			vertexBindingIndex,
			3,
			offsetof(
				Assets::MeshVertex,
				normal));

		m_VertexArray.SetFloatAttribute(
			4,
			vertexBindingIndex,
			4,
			offsetof(
				Assets::MeshVertex,
				tangent));
	}

	void GpuMesh::Bind() const noexcept
	{
		m_VertexArray.Bind();
	}

	std::size_t GpuMesh::IndexCount() const noexcept
	{
		return m_IndexCount;
	}
}