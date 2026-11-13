#pragma once
#include "ModelMaterialInspection.hpp"
#include "ModelNodeQuery.hpp"
#include "SceneSelection.hpp"
#include <map>
#include <memory>

#include "DirectionalShadowSettings.hpp"
#include "SandboxObject.hpp"
#include "SceneLight.hpp"
#include "ShaderFileMonitor.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <ForgeSim/Renderer/DebugGeometry.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/ColorRenderTarget.hpp>
#include <ForgeSim/Renderer/OpenGL/DebugLineRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/EnvironmentMapProcessor.hpp>
#include <ForgeSim/Renderer/OpenGL/FxaaRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/GpuMesh.hpp>
#include <ForgeSim/Renderer/OpenGL/HdrRenderTarget.hpp>
#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/ShadowMap.hpp>
#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>
#include <ForgeSim/Renderer/OpenGL/TextureCube.hpp>
#include <ForgeSim/Renderer/OpenGL/ToneMappingRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/SkyboxRenderer.hpp>

namespace ForgeSim::Sandbox
{
	struct SelectionHighlightSettings
	{
		bool enabled = true;

		glm::vec3 color{
			1.0f,
			0.35f,
			0.05f
		};

		float strength = 0.20f;
	};

	struct SandboxRenderSettings
	{
		std::uint32_t framebufferWidth = 0;
		std::uint32_t framebufferHeight = 0;

		glm::vec3 backgroundColor{ 0.0f };

		float exposure = 1.0f;
		float environmentIntensity = 0.2f;

		bool showEnvironmentBackground = false;
		bool showGrid = true;
		bool showObjectBounds = false;
		bool showLights = true;
        bool showDirectionalLightGuide = true;
		DirectionalShadowSettings directionalShadows{};
		Renderer::FxaaSettings fxaa{};

		std::optional<Renderer::AxisAlignedBoundingBox> lightDebugBounds;
		std::optional<SceneLightId> selectedLightId;

		SelectionHighlightSettings selectionHighlight;
		bool renderToTexture = false;
	};

	struct SandboxRendererSpecification
	{
		std::filesystem::path sceneFilename{
			"MetalRoughSpheresNoTextures.glb"
		};

		std::optional<std::filesystem::path>
			environmentFilename =
			std::filesystem::path{
				"Studio_Small_01_4k.hdr"
		};

		Assets::GltfTexturePolicy texturePolicy =
			Assets::GltfTexturePolicy::Strict;
		bool loadingDiagnostics = false;
		bool parallelImageDecode = true;
		bool cacheDecodedImages = false;
		std::uint64_t imageCacheBudgetBytes = Assets::DefaultImageCacheBudgetMiB * Assets::ImageCacheMiB;

		// Synchronous notifications on the thread that requested the load.
		// The observer must not reenter the renderer.
		std::function<void(const Assets::GltfLoadProgress&)> onLoadProgress;

		// Optional startup-only frame pump, called on the window thread.
		std::function<void(const Assets::GltfLoadProgress&)> drawStartupProgress;
	};

	class SandboxRenderer final
	{
	public:
		SandboxRenderer();

		explicit SandboxRenderer(
			const SandboxRendererSpecification& specification);

		SandboxRenderer(
			const SandboxRenderer&) = delete;

		SandboxRenderer& operator=(
			const SandboxRenderer&) = delete;

		SandboxRenderer(
			SandboxRenderer&&) = delete;

		SandboxRenderer& operator=(
			SandboxRenderer&&) = delete;

		void Render(
			std::span<const SandboxObject> objects,
			std::span<const SandboxObjectId> selectedObjectIds,
            const std::optional<ModelNodeSelection>& selectedNode,
			const glm::mat4& view,
			const glm::mat4& projection,
			const glm::vec3& cameraPosition,
			const SceneLightingView& light,
			const SandboxRenderSettings& settings);

		void ReloadChangedShaders();

        // Changes import policy for future model loads; existing GPU resources
        // remain active until a replacement scene/model is loaded successfully.
        void SetTexturePolicy(Assets::GltfTexturePolicy policy) noexcept { m_TexturePolicy = policy; }
        [[nodiscard]] Assets::GltfTexturePolicy TexturePolicy() const noexcept { return m_TexturePolicy; }

        // Preserve the configured disk budget and existing cache files.
        void SetDecodedImageCaching(bool enabled) noexcept { m_CacheDecodedImages = enabled; }
        [[nodiscard]] bool DecodedImageCaching() const noexcept { return m_CacheDecodedImages; }

        void SetLoadingDiagnostics(bool enabled) noexcept { m_LoadingDiagnostics = enabled; }
        [[nodiscard]] bool LoadingDiagnostics() const noexcept { return m_LoadingDiagnostics; }

        void LoadSceneModels(const std::filesystem::path& root,
            std::span<SandboxObject> objects,
            const std::optional<std::filesystem::path>& environmentPath,
            const std::function<void(const Assets::GltfLoadProgress&)>& drawProgress = {},
            std::optional<Assets::GltfTexturePolicy> texturePolicy = std::nullopt,
            bool reuseLoadedModels = true);

        // Prepare the environment and model before committing either replacement.
        // An absent model retains unused existing model resources for empty scenes.
        void LoadSceneResources(
            const std::optional<std::filesystem::path>& modelPath,
            const std::optional<std::filesystem::path>& environmentPath,
            const std::function<void(const Assets::GltfLoadProgress&)>& drawProgress = {},
            std::optional<Assets::GltfTexturePolicy> texturePolicy = std::nullopt);

		// Replace the shared model only after all replacement resources are ready.
		void LoadObjectModel(
			const std::filesystem::path& path,
			const std::function<void(const Assets::GltfLoadProgress&)>& drawProgress = {},
            std::optional<Assets::GltfTexturePolicy> texturePolicy = std::nullopt);

		// Returns zero until a frame has been rendered to the color target.
		[[nodiscard]]
		unsigned int SceneColorTexture() const noexcept;

		// Default model bounds, also used as an empty-scene fallback.
		[[nodiscard]]
		const Renderer::AxisAlignedBoundingBox&
			ObjectLocalBounds() const noexcept;

		[[nodiscard]]
		const std::filesystem::path& ObjectModelPath() const noexcept;

		void PrepareSceneTexture(
			std::uint32_t width,
			std::uint32_t height);

		void ClearWindow(
			std::uint32_t width,
			std::uint32_t height);

        [[nodiscard]] const ModelNodeQuery* InspectModelNodes(const std::string& modelReference) const;
        [[nodiscard]] const Assets::SceneAsset* InspectModelHierarchy(const std::string& modelReference) const;
        [[nodiscard]] ModelMaterialInspection InspectModelMaterials(const std::string& modelReference) const;

	private:

		void RenderDirectionalShadowMap(
			const glm::mat4& lightViewProjection, bool transmissionPass);

		struct GpuMaterialResource;
		struct GpuPrimitiveResource;
		void BindMaterial(const GpuMaterialResource* material);

		void AppendSceneNodeSubmissions(
			Assets::SceneNodeIndex nodeIndex,
			const glm::mat4& parentNodeTransform,
			const glm::mat4& objectTransform,
			bool isSelected, const Assets::SceneAsset& asset,
            const std::vector<GpuPrimitiveResource>& primitives,
            const std::vector<GpuMaterialResource>& materials,
            std::optional<std::size_t> selectedNode = std::nullopt,
            const NodeTransformOverrides& overrides = {});

		void UploadPointLights(
			std::span<const PointLight> lights);

		void UploadSpotLights(
			std::span<const SpotLight> lights);

		struct RenderSubmission
		{
			const Renderer::OpenGL::GpuMesh* mesh = nullptr;
			const GpuMaterialResource* material = nullptr;
			glm::mat4 modelTransform{ 1.0f };
			bool isSelected = false;
			bool castsDirectionalShadows = true;
		};

		struct GpuPrimitiveResource
		{
			Renderer::OpenGL::GpuMesh mesh;
			Assets::SceneMeshIndex sceneMeshIndex;
			std::optional<Assets::MaterialIndex> materialIndex;
		};

		struct GpuTextureResource
		{
			std::size_t sourceTextureIndex;
			Renderer::OpenGL::TextureColorSpace colorSpace;
			Renderer::OpenGL::Texture2D texture;
		};

		struct GpuTextureBinding
		{
			const Renderer::OpenGL::Texture2D* texture = nullptr;
			const Renderer::OpenGL::Sampler* sampler = nullptr;
			Assets::TextureTransformRows uvRows{{
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f }
			}};
		};

		struct GpuMaterialResource
		{
			GpuTextureBinding baseColor;
			GpuTextureBinding metallicRoughness;
			GpuTextureBinding normal;
			GpuTextureBinding occlusion;
			GpuTextureBinding emissive;

			glm::vec4 baseColorFactor{ 1.0f };
			glm::vec3 emissiveFactor{ 0.0f };

			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			float normalScale = 1.0f;
			float occlusionStrength = 1.0f;

			Assets::AlphaMode alphaMode =
				Assets::AlphaMode::Opaque;

			float alphaCutoff = 0.5f;
			bool doubleSided = false;
			std::optional<Assets::DirectionalTransmissionData> directionalTransmission;
		};

        struct ModelResources
        {
            Assets::SceneAsset asset;
            Renderer::AxisAlignedBoundingBox bounds;
            std::vector<GpuPrimitiveResource> primitives;
            std::vector<GpuTextureResource> textures;
            std::vector<Renderer::OpenGL::Sampler> samplers;
            std::vector<GpuMaterialResource> materials;
        };
        std::map<std::filesystem::path, std::shared_ptr<ModelResources>> m_Models;
        mutable std::map<const Assets::SceneAsset*, ModelNodeQuery> m_NodeQueries;
        std::map<std::string, std::filesystem::path> m_ModelBindings;

		void UploadObjectModelResources(
			const Assets::SceneAsset& sceneAsset,
			std::vector<GpuPrimitiveResource>& primitives,
			std::vector<GpuTextureResource>& textures,
			std::vector<Renderer::OpenGL::Sampler>& samplers,
			std::vector<GpuMaterialResource>& materials,
			const std::function<void(const Assets::GltfLoadProgress&)>& drawProgress = {});

		std::vector<RenderSubmission> m_RenderSubmissions;

        ShaderFileMonitor m_DirectionalShadowShaderMonitor;
        Renderer::OpenGL::ShaderProgram m_DirectionalShadowShaderProgram;
        Renderer::OpenGL::ShadowMap m_DirectionalShadowMap;
        std::optional<Renderer::OpenGL::ShadowMap> m_DirectionalTransmissionMap;
        glm::mat4 m_DirectionalLightViewProjection{ 1.0f };

		Assets::GltfTexturePolicy m_TexturePolicy;
		bool m_LoadingDiagnostics;
		const bool m_ParallelImageDecode;
		bool m_CacheDecodedImages;
		const std::uint64_t m_ImageCacheBudgetBytes;
		const std::function<void(const Assets::GltfLoadProgress&)> m_OnLoadProgress;
		std::filesystem::path m_ObjectModelPath;
        std::optional<std::filesystem::path> m_EnvironmentPath;
		Assets::SceneAsset m_ObjectSceneAsset;
		Renderer::AxisAlignedBoundingBox m_ObjectLocalBounds;

		ShaderFileMonitor m_ObjectShaderMonitor;
		Renderer::OpenGL::ShaderProgram m_ObjectShaderProgram;

		std::vector<GpuPrimitiveResource>
			m_GpuPrimitiveResources;

		std::vector<GpuTextureResource>
			m_GpuTextures;

		std::vector<Renderer::OpenGL::Sampler>
			m_GpuSamplers;

		std::vector<GpuMaterialResource>
			m_GpuMaterialResources;

		Renderer::OpenGL::Texture2D m_FallbackWhiteTexture;
		Renderer::OpenGL::Texture2D m_FallbackNormalTexture;
		Renderer::OpenGL::Sampler m_DefaultSampler;

		Renderer::OpenGL::Texture2D
			m_EnvironmentEquirectangularTexture;

		Renderer::OpenGL::EnvironmentMapProcessor
			m_EnvironmentMapProcessor;

		Renderer::OpenGL::TextureCube
			m_EnvironmentCubemap;

		ShaderFileMonitor m_SkyboxShaderMonitor;

		Renderer::OpenGL::SkyboxRenderer
			m_SkyboxRenderer;

		Renderer::OpenGL::TextureCube
			m_DiffuseIrradianceMap;

		Renderer::OpenGL::TextureCube
			m_PrefilteredSpecularMap;

		Renderer::OpenGL::Texture2D
			m_BrdfLookupTexture;

		std::vector<ForgeSim::Renderer::DebugLineVertex>
			m_GridVertices;

		std::vector<Renderer::DebugLineVertex>
			m_BoundsVertices;

		std::vector<Renderer::DebugLineVertex> m_LightVertices;

		ShaderFileMonitor m_GridShaderMonitor;
		ForgeSim::Renderer::OpenGL::DebugLineRenderer
			m_DebugLineRenderer;

		std::optional<Renderer::OpenGL::HdrRenderTarget>
			m_HdrRenderTarget;

		std::optional<Renderer::OpenGL::ColorRenderTarget>
			m_ColorRenderTarget;

		ShaderFileMonitor m_ToneMappingShaderMonitor;

		Renderer::OpenGL::ToneMappingRenderer
			m_ToneMappingRenderer;

        // ImGui always displays m_ColorRenderTarget, even when FXAA is toggled.
        std::optional<Renderer::OpenGL::ColorRenderTarget> m_FxaaSource;
        ShaderFileMonitor m_FxaaShaderMonitor;
        Renderer::OpenGL::FxaaRenderer m_FxaaRenderer;
	};
}
