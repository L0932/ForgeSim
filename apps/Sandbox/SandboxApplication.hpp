#pragma once

#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Sandbox
{
	struct SandboxApplicationSpecification
	{
		Platform::WindowSpecification windowSpec;
	};

	class SandboxApplication final
	{
	public:
		explicit SandboxApplication(
			const SandboxApplicationSpecification& spec
		);

		SandboxApplication(const SandboxApplication&) = delete;
		SandboxApplication& operator=(const SandboxApplication&) = delete;

		SandboxApplication(SandboxApplication&&) = delete;
		SandboxApplication& operator=(SandboxApplication&&) = delete;

		void Run(); // owns the outer interactive loop

	private:
		Platform::GlfwWindow m_Window;

		ForgeSim::Renderer::OpenGL::ShaderProgram m_ObjectShaderProgram;
		ForgeSim::Renderer::OpenGL::ShaderProgram m_GridShaderProgram;

		ForgeSim::Renderer::OpenGL::Buffer m_VertexBuffer;
		ForgeSim::Renderer::OpenGL::Buffer m_IndexBuffer;
		ForgeSim::Renderer::OpenGL::VertexArray m_VertexArray;

		ForgeSim::Renderer::OpenGL::Buffer m_GridVertexBuffer;
		ForgeSim::Renderer::OpenGL::VertexArray m_GridVertexArray;

		bool m_RunAttempted = false;
	};
}