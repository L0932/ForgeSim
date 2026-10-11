#pragma once

#include "SandboxObject.hpp"

#include <cstddef>
#include <optional>
#include <span>

#include <glm/mat4x4.hpp>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
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
			const glm::mat4& projection);

	private:
		Assets::MeshData m_ObjectMesh;

		Renderer::OpenGL::ShaderProgram m_ObjectShaderProgram;
		Renderer::OpenGL::ShaderProgram m_GridShaderProgram;

		Renderer::OpenGL::Buffer m_VertexBuffer;
		Renderer::OpenGL::Buffer m_IndexBuffer;
		Renderer::OpenGL::VertexArray m_VertexArray;

		Renderer::OpenGL::Buffer m_GridVertexBuffer;
		Renderer::OpenGL::VertexArray m_GridVertexArray;

		std::size_t m_GridVertexCount = 0;
	};
}