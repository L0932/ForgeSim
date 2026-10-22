#include "ViewportTransformGizmo.hpp"

#include <cmath>
#include <stdexcept>

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

namespace ForgeSim::Sandbox
{
	namespace
	{
		[[nodiscard]] ImGuizmo::OPERATION ToOperation(
			GizmoOperation operation)
		{
			switch (operation)
			{
			case GizmoOperation::Translate:
				return ImGuizmo::TRANSLATE;

			case GizmoOperation::Rotate:
				return ImGuizmo::ROTATE;

			case GizmoOperation::Scale:
				return ImGuizmo::SCALE;
			}

			throw std::invalid_argument(
				"Unsupported gizmo operation.");
		}

		[[nodiscard]] ImGuizmo::MODE ToMode(
			GizmoSpace space)
		{
			switch (space)
			{
			case GizmoSpace::Local:
				return ImGuizmo::LOCAL;

			case GizmoSpace::World:
				return ImGuizmo::WORLD;
			}

			throw std::invalid_argument(
				"Unsupported gizmo coordinate space.");
		}
	}

	void ViewportTransformGizmo::BeginFrame()
	{
		ImGuizmo::BeginFrame();
	}

	GizmoInteraction ViewportTransformGizmo::Draw(
		const ViewportTransformGizmoContext& context)
	{
		if (!std::isfinite(context.viewportPosition.x) ||
			!std::isfinite(context.viewportPosition.y) ||
			!std::isfinite(context.viewportSize.x) ||
			!std::isfinite(context.viewportSize.y) ||
			context.viewportSize.x <= 0.0f ||
			context.viewportSize.y <= 0.0f)
		{
			return {};
		}

		const ImGuizmo::OPERATION operation =
			ToOperation(context.operation);

		// Object-axis scaling uses local coordinates.
		const ImGuizmo::MODE mode =
			context.operation == GizmoOperation::Scale
			? ImGuizmo::LOCAL
			: ToMode(context.space);

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::Enable(context.enabled);

		// Draw over the scene, beneath ordinary ImGui windows.
		ImGuizmo::SetDrawlist(
			ImGui::GetWindowDrawList());

		ImGuizmo::SetRect(
			context.viewportPosition.x,
			context.viewportPosition.y,
			context.viewportSize.x,
			context.viewportSize.y);

		const bool changed =
			ImGuizmo::Manipulate(
				glm::value_ptr(context.view),
				glm::value_ptr(context.projection),
				operation,
				mode,
				glm::value_ptr(context.transform));

		return GizmoInteraction{
			.changed = context.enabled && changed,
			.hovered = context.enabled && ImGuizmo::IsOver(),
			.active = context.enabled && ImGuizmo::IsUsing()
		};
	}

	GizmoInteraction ViewportTransformGizmo::DrawInMainViewport(
		const ViewportTransformGizmoContext& context,
		bool continueInteraction)
	{
		const ImGuiViewport* viewport =
			ImGui::GetMainViewport();

		const bool pointerOverUI =
			ImGui::IsWindowHovered(
				ImGuiHoveredFlags_AnyWindow);

		ViewportTransformGizmoContext viewportContext = context;

		viewportContext.viewportPosition = {
			viewport->Pos.x,
			viewport->Pos.y
		};

		viewportContext.viewportSize = {
			viewport->Size.x,
			viewport->Size.y
		};

		viewportContext.enabled =
			context.enabled &&
			(continueInteraction || !pointerOverUI);

		return Draw(viewportContext);
	}
}