#pragma once

#include "SandboxDebugUI.hpp"
#include "SandboxRenderer.hpp"
#include "SandboxScene.hpp"
#include "SceneBounds.hpp"
#include "SceneDocument.hpp"

#include <filesystem>
#include <optional>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Sandbox
{
	struct SandboxApplicationSpecification
	{
		Platform::WindowSpecification windowSpec;
		SandboxRendererSpecification rendererSpec;

		std::optional<SceneDocument> initialScene;
		std::optional<std::filesystem::path> projectRoot;
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

		void SaveScene(const std::filesystem::path& path) const;

	private:
		Platform::GlfwWindow m_Window;
		SandboxScene m_Scene;
		SandboxRenderer m_Renderer;
		SandboxDebugUI m_DebugUI;
		std::filesystem::path m_ProjectRoot;
		
		bool m_LoadedScene = false;
		bool m_RunAttempted = false;
	};
}