#pragma once

#include <span>
#include <string_view>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class ShaderProgram final
	{
	public:
		explicit ShaderProgram(
			std::string_view vertexShaderSource,
			std::string_view fragmentShaderSource,
			std::span<const std::string_view>
				requiredUniformNames);

		~ShaderProgram();

		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;

		ShaderProgram(ShaderProgram&& other) noexcept;
		ShaderProgram& operator=(ShaderProgram&& other) noexcept;

		void Bind() const noexcept;

		void SetFloat(
			std::string_view name,
			float value) const;

		void SetInt(
			std::string_view name,
			int value
		) const;

		void SetVector3(
			std::string_view name,
			const glm::vec3& value) const;

		void SetVector4(
			std::string_view name,
			const glm::vec4& value) const;

		void SetMatrix4x4(
			std::string_view name,
			const glm::mat4& value) const;

	private:
		void ValidateRequiredUniforms(
			std::span<const std::string_view>
			requiredUniformNames) const;

		unsigned int m_Handle = 0;
	};
}