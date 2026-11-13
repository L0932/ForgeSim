#pragma once 

#pragma once
#include "LightPicking.hpp"
#include <functional>
#include "SandboxObject.hpp"
#include "SceneSelection.hpp"

#include <span>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
	class ViewportSelectionController final
	{
	public:
		void Update(
			const Platform::InputState& input,
			glm::vec2 viewportCursorPixels,
			Platform::FramebufferExtent framebufferExtent,
			const glm::mat4& view,
			const glm::mat4& projection,
			const Renderer::AxisAlignedBoundingBox& localBounds,
			std::span<const SandboxObject> objects,
			std::span<const LightPickTarget> lightTargets,
			SceneSelection& selection,
			bool pickingSuppressed = false, bool ctrl = false, bool shift = false,
            bool selectNode = false,
            const std::function<std::optional<ModelNodeHit>(const Renderer::Ray&, const SandboxObject&)>& pickNode = {});
	};
}