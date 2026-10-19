#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace ForgeSim::Sandbox
{
	enum class GizmoOperation
	{
		Translate,
		Rotate,
		Scale
	};

	enum class GizmoSpace
	{
		Local,
		World
	};

	struct ViewportTransformGizmoContext
	{
		const glm::mat4& view;
		const glm::mat4& projection;
		glm::mat4& transform;

		// ImGui screen coordinates, not framebuffer pixels.
		glm::vec2 viewportPosition{ 0.0f };
		glm::vec2 viewportSize{ 0.0f };

		GizmoOperation operation = GizmoOperation::Translate;
		GizmoSpace space = GizmoSpace::Local;

		bool enabled = true;
	};

	struct GizmoInteraction
	{
		bool changed = false;
		bool hovered = false;
		bool active = false;
	};

	class ViewportTransformGizmo final
	{
	public:
		// Call once per frame after ImGui::NewFrame().
		void BeginFrame();

		[[nodiscard]] GizmoInteraction Draw(
			const ViewportTransformGizmoContext& context);

		[[nodiscard]] GizmoInteraction DrawInMainViewport(
			const ViewportTransformGizmoContext& context,
			bool continueInteraction);
	};
}