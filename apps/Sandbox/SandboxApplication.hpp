#pragma once

#include "EditorSession.hpp"
#include "SandboxDebugUI.hpp"
#include "SandboxRenderer.hpp"
#include "SandboxScene.hpp"
#include "SceneBounds.hpp"
#include "SceneDocument.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

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
        std::optional<std::filesystem::path> explicitSceneRoot;
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

		void Run(std::optional<std::chrono::steady_clock::time_point> startupStart); // owns the outer interactive loop

		static void ShowStartupError(const Platform::WindowSpecification& specification,
			std::string_view message);

		void SaveScene(const std::filesystem::path& path);

		[[nodiscard]]
		std::optional<std::filesystem::path> ChooseScenePath(bool saving);

		[[nodiscard]] bool ShouldSaveOnExit() const noexcept
		{
			return m_SaveOnExit;
		}

	private:
		Platform::GlfwWindow m_Window;
		SandboxDebugUI m_DebugUI;
		SandboxRenderer m_Renderer;
		std::unique_ptr<EditorSession> m_EditorSession;

		bool m_LoadedScene = false;
        std::optional<std::filesystem::path> m_ExplicitSceneRoot;
		bool m_RunAttempted = false;
		bool m_SaveOnExit = true;
	};
}