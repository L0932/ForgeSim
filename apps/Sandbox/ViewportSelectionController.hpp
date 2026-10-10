#pragma once 

#include "SandboxObject.hpp"

#include <optional>
#include <span>

#include <glm/mat4x4.hpp>

#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Platform/InputState.hpp>

namespace ForgeSim::Sandbox
{
	class ViewportSelectionController final
	{
	public:
		void Update(
			const Platform::InputState& input,
			Platform::WindowExtent windowExtent,
			Platform::FramebufferExtent framebufferExtent,
			const glm::mat4& view,
			const glm::mat4& projection,
			std::span<const SandboxObject> objects);

		[[nodiscard]] std::optional<SandboxObjectId>
			SelectedObjectId() const noexcept;

		[[nodiscard]] bool IsSelected(
			SandboxObjectId objectId) const noexcept;

		void Clear() noexcept;

	private:
		std::optional<SandboxObjectId> m_SelectedObjectId;
	};
}