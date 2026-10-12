#pragma once

#include <span>
#include <string_view>

#include <glm/mat4x4.hpp>

#include <ForgeSim/Renderer/DebugGeometry.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class DebugLineRenderer final
	{
	public:
		DebugLineRenderer(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

		DebugLineRenderer(
			const DebugLineRenderer&) = delete;

		DebugLineRenderer& operator=(
			const DebugLineRenderer&) = delete;

		DebugLineRenderer(
			DebugLineRenderer&&) = delete;

		DebugLineRenderer& operator=(
			DebugLineRenderer&&) = delete;

		void Render(
			std::span<const DebugLineVertex> vertices,
			const glm::mat4& view,
			const glm::mat4& projection);

		void ReloadShaders(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

	private:
		ShaderProgram m_ShaderProgram;

		Buffer m_VertexBuffer;
		VertexArray m_VertexArray;
	};
}