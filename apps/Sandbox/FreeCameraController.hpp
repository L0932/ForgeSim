#pragma once

#include <optional>

#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
	struct FreeCameraControllerSpecification
	{
		float movementSpeed = 2.5f;
		float mouseSensitivity = 0.002f;
		float maximumDeltaSeconds = 0.1f;
	};

	class FreeCameraController final
	{
	public:
		explicit FreeCameraController(
			const FreeCameraControllerSpecification&
			specification = {}) noexcept;

		[[nodiscard]]
		std::optional<Platform::CursorMode> RequestedCursorMode(
			const Platform::InputState& input) const noexcept;

		void Update(
			ForgeSim::Renderer::PerspectiveCamera& camera,
			const ForgeSim::Platform::InputState& input,
			float deltaSeconds) const;

	private:
		FreeCameraControllerSpecification m_Specification;
	};
}