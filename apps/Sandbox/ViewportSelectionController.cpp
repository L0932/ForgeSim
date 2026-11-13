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
		glm::vec2 viewportCursorPixels,
		Platform::FramebufferExtent framebufferExtent,
		const glm::mat4& view,
		const glm::mat4& projection,
		const Renderer::AxisAlignedBoundingBox& localBounds,
		std::span<const SandboxObject> objects,
		std::span<const LightPickTarget> lightTargets,
		SceneSelection& selection,
		bool pickingSuppressed, bool ctrl, bool shift, bool selectNode,
        const std::function<std::optional<ModelNodeHit>(const Renderer::Ray&, const SandboxObject&)>& pickNode)
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

		if (!framebufferExtent.IsDrawable() ||
			!std::isfinite(viewportCursorPixels.x) ||
			!std::isfinite(viewportCursorPixels.y) ||
			viewportCursorPixels.x < 0.0f ||
			viewportCursorPixels.y < 0.0f ||
			viewportCursorPixels.x >=
			static_cast<float>(framebufferExtent.width) ||
			viewportCursorPixels.y >=
			static_cast<float>(framebufferExtent.height))
		{
			return;
		}

		const Renderer::Ray pickingRay =
			Renderer::CreateViewportRay(
				static_cast<double>(viewportCursorPixels.x),
				static_cast<double>(viewportCursorPixels.y),
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

        if (selectNode && pickNode)
        {
            const SandboxObject* owner = nullptr;
            std::optional<ModelNodeHit> closest;
            for (const auto& object : objects)
            {
                const auto hit = pickNode(pickingRay, object);
                if (hit && (!closest || hit->distance < closest->distance)) { owner = &object; closest = hit; }
            }
            if (owner) selection.SelectNode(owner->id, closest->nodeIndex, owner->modelPath);
            else selection.Clear();
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
					object.localBounds.value_or(localBounds),
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
			selection.ClickObject(*closestObjectId, ctrl, shift);
		}
		else if (!ctrl && !shift)
		{
			selection.Clear();
		}
	}
}