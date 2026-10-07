#pragma once

#include <string_view>

#include <glm/mat4x4.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class ShaderProgram final
	{
	public:
		explicit ShaderProgram(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource
		);

		~ShaderProgram();

		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;

		ShaderProgram(ShaderProgram&&) = delete;
		ShaderProgram& operator=(ShaderProgram&&) = delete;

		void Bind() const noexcept;

		void SetMatrix4x4(
			std::string_view name,
			const glm::mat4& value) const;

	private:
		unsigned int m_Handle = 0;
	};
}