#pragma once

#include "SandboxObject.hpp"
#include "ShaderFileMonitor.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include <glm/mat4x4.hpp>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Renderer/DebugGeometry.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/DebugLineRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Sandbox
{
	class SandboxRenderer final
	{
	public:
		SandboxRenderer();

		SandboxRenderer(
			const SandboxRenderer&) = delete;

		SandboxRenderer& operator=(
			const SandboxRenderer&) = delete;

		SandboxRenderer(
			SandboxRenderer&&) = delete;

		SandboxRenderer& operator=(
			SandboxRenderer&&) = delete;

		void Render(
			std::span<const SandboxObject> objects,
			std::optional<SandboxObjectId> selectedObjectId,
			const glm::mat4& view,
			const glm::mat4& projection,
			bool showGrid,
			bool showObjectBounds);

		void ReloadChangedShaders();

		// Temporary bridge while SandboxRenderer owns the shared object mesh.
		// Move bounds ownership to the mesh resource when objects support distinct meshes.
		[[nodiscard]]
		const Renderer::AxisAlignedBoundingBox&
			ObjectLocalBounds() const noexcept;

	private:
		Assets::MeshData m_ObjectMesh;
		Renderer::AxisAlignedBoundingBox m_ObjectLocalBounds;

		ShaderFileMonitor m_ObjectShaderMonitor;
		Renderer::OpenGL::ShaderProgram m_ObjectShaderProgram;

		Renderer::OpenGL::Buffer m_VertexBuffer;
		Renderer::OpenGL::Buffer m_IndexBuffer;
		Renderer::OpenGL::VertexArray m_VertexArray;

		std::vector<ForgeSim::Renderer::DebugLineVertex>
			m_GridVertices;

		std::vector<Renderer::DebugLineVertex>
			m_BoundsVertices;

		ShaderFileMonitor m_GridShaderMonitor;
		ForgeSim::Renderer::OpenGL::DebugLineRenderer
			m_DebugLineRenderer;
	};
}