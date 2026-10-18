#include "LightPicking.hpp"
#include "SandboxObject.hpp"
#include "SceneSelection.hpp"
#include "ViewportSelectionController.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <span>

#include <ForgeSim/Platform/GlfwWindow.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/Picking.hpp>

namespace ForgeSim::Sandbox
{
	void ViewportSelectionController::Update(
		const Platform::InputState& input,
		Platform::WindowExtent windowExtent,
		Platform::FramebufferExtent framebufferExtent,
		const glm::mat4& view,
		const glm::mat4& projection,
		const Renderer::AxisAlignedBoundingBox& localBounds,
		std::span<const SandboxObject> objects,
		std::span<const LightPickTarget> lightTargets,
		SceneSelection& selection,
		bool pickingSuppressed)
	{
		if (pickingSuppressed)
		{
			return;
		}

		const auto leftMouseState =
			input.GetMouseButtonState(
				Platform::MouseButton::Left);

		const auto rightMouseState =
			input.GetMouseButtonState(
				Platform::MouseButton::Right);

		const bool cameraLookIsActive =
			rightMouseState.pressed ||
			rightMouseState.held;

		if (!leftMouseState.pressed ||
			cameraLookIsActive)
		{
			return;
		}

		if (!windowExtent.IsValid() ||
			!framebufferExtent.IsDrawable())
		{
			return;
		}

		const auto cursorPosition =
			input.GetCursorPosition();

		const double framebufferCursorX =
			cursorPosition.x *
			static_cast<double>(framebufferExtent.width) /
			static_cast<double>(windowExtent.width);

		const double framebufferCursorY =
			cursorPosition.y *
			static_cast<double>(framebufferExtent.height) /
			static_cast<double>(windowExtent.height);

		const Renderer::Ray pickingRay =
			Renderer::CreateViewportRay(
				framebufferCursorX,
				framebufferCursorY,
				framebufferExtent.width,
				framebufferExtent.height,
				view,
				projection);

		// Light markers are debug overlays and take precedence over
		// object bounds, including when geometry lies in front of them.
		if (const auto lightHit =
			PickClosestLight(pickingRay, lightTargets))
		{
			selection.SelectLight(lightHit->id);
			return;
		}

		std::optional<SandboxObjectId> closestObjectId;
		float closestDistance =
			std::numeric_limits<float>::max();

		for (const SandboxObject& object : objects)
		{						
			const auto hitDistance =
				IntersectRayTransformedBox(
					pickingRay,
					localBounds,
					object.transform.ModelMatrix());

			if (hitDistance &&
				*hitDistance < closestDistance)
			{
				closestDistance = *hitDistance;
				closestObjectId = object.id;
			}
		}

		if (closestObjectId)
		{
			selection.SelectObject(*closestObjectId);
		}
		else
		{
			selection.Clear();
		}
	}
}