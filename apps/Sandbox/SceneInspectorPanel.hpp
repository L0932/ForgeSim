#pragma once
#include "ModelMaterialInspection.hpp"
#include "SceneEditHistory.hpp"
#include "ModelNodeQuery.hpp"

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
    class SceneTransformEditor;

	struct SceneInspectorPanelContext
	{
		SandboxScene& scene;
		SceneSelection& selection;
		const Renderer::AxisAlignedBoundingBox& modelBounds;

		bool editingDisabled = false;
        SceneTransformEditor* transformEditor = nullptr;
        ModelMaterialInspection materials;
        const ModelNodeQuery* nodeQuery = nullptr;
	};

	class SceneInspectorPanel final
	{
	public:
		void Draw(const SceneInspectorPanelContext& context);
		void Reset() noexcept;

	private:
		void DrawLights(const SceneInspectorPanelContext& context);
		void DrawObject(const SceneInspectorPanelContext& context);
        void DrawNode(const SceneInspectorPanelContext& context);
        std::optional<ObjectBatchEdit> m_ObjectEdit;
        std::vector<SandboxObjectId> m_EditTargets;
        void FinishObjectEdit(SceneTransformEditor* editor);
        void DrawMaterials(const ModelMaterialInspection& inspection);

		[[nodiscard]] static float CalculatePositionDragSpeed(
			const Renderer::AxisAlignedBoundingBox& bounds);

		std::optional<LightValidationError> m_LightValidationError;
	};
}