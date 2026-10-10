#include "ViewportSelectionController.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <ForgeSim/Renderer/Picking.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		constexpr Renderer::AxisAlignedBoundingBox unitCubeBounds{
			.minimum = glm::vec3{ -0.5f },
			.maximum = glm::vec3{ 0.5f }
		};
	}

	void ViewportSelectionController::Update(
		const Platform::InputState& input,
		Platform::WindowExtent windowExtent,
		Platform::FramebufferExtent framebufferExtent,
		const glm::mat4& view,
		const glm::mat4& projection,
		std::span<const SandboxObject> objects)
	{
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

		std::optional<SandboxObjectId> closestObjectId;
		float closestDistance =
			std::numeric_limits<float>::max();

		for (const SandboxObject& object : objects)
		{						
			const auto hitDistance =
				IntersectRayTransformedBox(
					pickingRay,
					unitCubeBounds,
					object.transform.ModelMatrix());

			if (hitDistance &&
				*hitDistance < closestDistance)
			{
				closestDistance = *hitDistance;
				closestObjectId = object.id;
			}
		}

		// An empty click intentionally clears the selection.
		m_SelectedObjectId = closestObjectId;
	}

	std::optional<SandboxObjectId>
		ViewportSelectionController::SelectedObjectId()
		const noexcept
	{
		return m_SelectedObjectId;
	}

	bool ViewportSelectionController::IsSelected(
		SandboxObjectId objectId) const noexcept
	{
		return m_SelectedObjectId &&
			*m_SelectedObjectId == objectId;
	}

	void ViewportSelectionController::Clear() noexcept
	{
		m_SelectedObjectId.reset();
	}
}