#pragma once

#include <ForgeSim/Platform/GlfwWindow.hpp>

namespace ForgeSim::Sandbox
{
	struct SandboxApplicationSpecification
	{
		Platform::WindowSpecification windowSpec;
	};

	class SandboxApplication
	{
	public:
		explicit SandboxApplication(
			const SandboxApplicationSpecification& spec
		);

		void Run(); // owns the outer interactive loop

	private:
		Platform::GlfwWindow m_Window;
		bool m_RunAttempted = false;
	};
}