#pragma once
#include "ModelMaterialInspection.hpp"
#include "SceneEditHistory.hpp"
#include "ModelNodeQuery.hpp"

#include "SceneLightValidation.hpp"

#include <optional>
#include <string>

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
        std::string m_NodeEditError;
        std::optional<ObjectBatchEdit> m_ObjectEdit;
        std::vector<SandboxObjectId> m_EditTargets;
        void FinishObjectEdit(SceneTransformEditor* editor);
        void DrawMaterials(const ModelMaterialInspection& inspection);
        void DrawTextureViewer(const ModelMaterialInspection& inspection);
        struct TextureViewer
        {
            const Assets::SceneAsset* asset = nullptr; // Identity only; never dereferenced across frames.
            std::uint64_t generation = 0;
            std::size_t imageIndex = 0;
            TexturePreviewMode mode = TexturePreviewMode::Rgb;
            bool packed = false;
            bool fit = true;
            float zoom = 1.0f;
            float panX = 0.0f;
            float panY = 0.0f;
            std::string name;
        };
        std::optional<TextureViewer> m_TextureViewer;

		[[nodiscard]] static float CalculatePositionDragSpeed(
			const Renderer::AxisAlignedBoundingBox& bounds);

		std::optional<LightValidationError> m_LightValidationError;
	};
}