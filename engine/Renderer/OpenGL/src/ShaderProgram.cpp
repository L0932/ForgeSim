#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>

#include <stdexcept>
#include <string>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{ 
	namespace
	{
		[[nodiscard]] std::string GetShaderInfoLog(
			unsigned int shader)
		{
			int infoLogLength = 0;

			glGetShaderiv(
				shader, 
				GL_INFO_LOG_LENGTH, 
				&infoLogLength);

			if (infoLogLength <= 1)
			{
				return {};
			}
			
			std::string infoLog(
				static_cast<std::size_t>(infoLogLength), 
				'\0');

			int writtenLength = 0;

			glGetShaderInfoLog(
				shader,
				infoLogLength,
				&writtenLength,
				infoLog.data());

			infoLog.resize(
				static_cast<std::size_t>(writtenLength));

			return infoLog;
		}

		[[nodiscard]] std::string GetProgramInfoLog(
			unsigned int program)
		{
			int infoLogLength = 0;

			glGetProgramiv(
				program, 
				GL_INFO_LOG_LENGTH, 
				&infoLogLength);

			if (infoLogLength <= 1)
			{
				return {};
			}

			std::string infoLog(
				static_cast<std::size_t>(infoLogLength), 
				'\0');

			int writtenLength = 0;

			glGetProgramInfoLog(
				program,
				infoLogLength,
				&writtenLength,
				infoLog.data());

			infoLog.resize(
				static_cast<std::size_t>(writtenLength));

			return infoLog;
		}

		[[nodiscard]] unsigned int CompileShader(
			unsigned int shaderType,
			std::string_view shaderSource,
			std::string_view stageName) 
		{
			if (shaderSource.size() >
				static_cast<std::size_t>(
					std::numeric_limits<int>::max()))
			{
				throw std::length_error(
					"Shader source exceeds the supported length");
			}

			const unsigned int shader =
				glCreateShader(shaderType);

			if (shader == 0)
			{
				throw std::runtime_error(
					"Failed to create " +
					std::string{ stageName });
			}

			const char* sourceData =
				shaderSource.data();

			const int sourceLength =
				static_cast<int>(shaderSource.size());

			glShaderSource(
				shader,
				1,
				&sourceData,
				&sourceLength);

			glCompileShader(shader);

			int compilationSucceeded = GL_FALSE;

			glGetShaderiv(
				shader,
				GL_COMPILE_STATUS,
				&compilationSucceeded);

			if (compilationSucceeded != GL_TRUE)
			{
				std::string infoLog;

				try
				{
					infoLog = GetShaderInfoLog(shader);
				}
				catch (...)
				{
					glDeleteShader(shader);
					throw;
				}

				glDeleteShader(shader);

				throw std::runtime_error(
					std::string{ stageName } +
					" compilation failed:\n" +
					infoLog);
			}

			return shader;
		}
	}

	ShaderProgram::ShaderProgram(
		std::string_view vertexShaderSource,
		std::string_view fragmentShaderSource)
	{
		unsigned int vertexShader = 0;
		unsigned int fragmentShader = 0;

		try
		{
			vertexShader = CompileShader(
				GL_VERTEX_SHADER,
				vertexShaderSource,
				"Vertex shader");

			fragmentShader = CompileShader(
				GL_FRAGMENT_SHADER,
				fragmentShaderSource,
				"Fragment shader");

			m_Handle = glCreateProgram();

			if (m_Handle == 0)
			{
				throw std::runtime_error(
					"Failed to create OpenGL shader program");
			}

			glAttachShader(
				m_Handle,
				vertexShader);

			glAttachShader(
				m_Handle,
				fragmentShader);

			glLinkProgram(m_Handle);

			int linkingSucceeded = GL_FALSE;

			glGetProgramiv(
				m_Handle,
				GL_LINK_STATUS,
				&linkingSucceeded);

			if (linkingSucceeded != GL_TRUE)
			{
				const std::string infoLog =
					GetProgramInfoLog(m_Handle);

				throw std::runtime_error(
					"Shader program linking failed:\n" +
					infoLog);
			}

			glDetachShader(
				m_Handle,
				vertexShader);

			glDetachShader(
				m_Handle,
				fragmentShader);

			glDeleteShader(fragmentShader);
			fragmentShader = 0;

			glDeleteShader(vertexShader);
			vertexShader = 0;
		}
		catch (...)
		{
			if (m_Handle != 0)
			{
				glDeleteProgram(m_Handle);
				m_Handle = 0;
			}

			if (fragmentShader != 0)
			{
				glDeleteShader(fragmentShader);
			}

			if (vertexShader != 0)
			{
				glDeleteShader(vertexShader);
			}

			throw;
		}
	}

	ShaderProgram::~ShaderProgram()
	{
		if (m_Handle != 0)
		{
			glDeleteProgram(m_Handle);
			m_Handle = 0;
		}
	}

	void ShaderProgram::Bind() const noexcept
	{
		glUseProgram(m_Handle);
	}
}