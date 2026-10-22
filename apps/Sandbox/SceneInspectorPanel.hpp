#pragma once

#include "SceneLightValidation.hpp"

#include <optional>

namespace ForgeSim::Renderer
{
	struct AxisAlignedBoundingBox;
}

namespace ForgeSim::Sandbox
{
	class SandboxScene;
	class SceneSelection;

	struct SceneInspectorPanelContext
	{
		SandboxScene& scene;
		SceneSelection& selection;
		const Renderer::AxisAlignedBoundingBox& modelBounds;

		bool editingDisabled = false;
	};

	class SceneInspectorPanel final
	{
	public:
		void Draw(const SceneInspectorPanelContext& context);
		void Reset() noexcept;

	private:
		void DrawLights(const SceneInspectorPanelContext& context);
		void DrawObject(const SceneInspectorPanelContext& context);

		[[nodiscard]] static float CalculatePositionDragSpeed(
			const Renderer::AxisAlignedBoundingBox& bounds);

		std::optional<LightValidationError> m_LightValidationError;
	};
}