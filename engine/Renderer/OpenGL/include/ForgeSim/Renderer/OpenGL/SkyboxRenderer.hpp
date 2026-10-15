#pragma once

#include <string_view>

#include <glm/mat4x4.hpp>

#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class TextureCube;

	class SkyboxRenderer final
	{
	public:
		SkyboxRenderer(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

		SkyboxRenderer(
			const SkyboxRenderer&) = delete;

		SkyboxRenderer& operator=(
			const SkyboxRenderer&) = delete;

		SkyboxRenderer(
			SkyboxRenderer&&) = delete;

		SkyboxRenderer& operator=(
			SkyboxRenderer&&) = delete;

		void Render(
			const TextureCube& environmentMap,
			const glm::mat4& view,
			const glm::mat4& projection,
			float intensity);

		void ReloadShaders(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource);

	private:
		ShaderProgram m_ShaderProgram;

		Buffer m_VertexBuffer;
		Buffer m_IndexBuffer;
		VertexArray m_VertexArray;
	};
}