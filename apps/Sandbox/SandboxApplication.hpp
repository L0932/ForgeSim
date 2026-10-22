#pragma once

#include "EditorSession.hpp"
#include "SandboxDebugUI.hpp"
#include "SandboxRenderer.hpp"
#include "SandboxScene.hpp"
#include "SceneBounds.hpp"
#include "SceneDocument.hpp"

#include <filesystem>
#include <memory>
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
		std::optional<std::filesystem::path> scenePath;
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

		void SaveScene(const std::filesystem::path& path);

		[[nodiscard]]
		std::optional<std::filesystem::path> ChooseScenePath(bool saving);

		[[nodiscard]] bool ShouldSaveOnExit() const noexcept
		{
			return m_SaveOnExit;
		}

	private:
		Platform::GlfwWindow m_Window;
		SandboxRenderer m_Renderer;
		SandboxDebugUI m_DebugUI;
		std::unique_ptr<EditorSession> m_EditorSession;

		bool m_LoadedScene = false;
		bool m_RunAttempted = false;
		bool m_SaveOnExit = true;
	};
}