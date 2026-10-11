#include "SandboxDebugUI.hpp"

#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ForgeSim/Platform/GlfwWindow.hpp>

namespace ForgeSim::Sandbox
{
	SandboxDebugUI::SandboxDebugUI(
		Platform::GlfwWindow& window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		if (!ImGui_ImplGlfw_InitForOpenGL(
			window.NativeHandle(),
			true))
		{
			ImGui::DestroyContext();

			throw std::runtime_error(
				"Failed to initialize the Dear ImGui "
				"GLFW backend.");
		}

		if (!ImGui_ImplOpenGL3_Init(
			"#version 460 core"))
		{
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			throw std::runtime_error(
				"Failed to initialize the Dear ImGui "
				"OpenGL backend.");
		}
	}

	SandboxDebugUI::~SandboxDebugUI()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void SandboxDebugUI::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::IsKeyPressed(
			ImGuiKey_F1,
			false))
		{
			m_IsVisible = !m_IsVisible;
		}
	}

	void SandboxDebugUI::Draw()
	{
		if (!m_IsVisible)
		{
			return;
		}

		ImGui::Begin(
			"ForgeSim Debug",
			&m_IsVisible);

		ImGui::TextUnformatted(
			"ForgeSim Sandbox debug interface");

		ImGui::TextUnformatted(
			"Press F1 to show or hide this panel.");

		ImGui::End();
	}

	void SandboxDebugUI::EndFrame()
	{
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData());
	}

	bool SandboxDebugUI::WantsMouseCapture()
		const noexcept
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool SandboxDebugUI::WantsKeyboardCapture()
		const noexcept
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}
}