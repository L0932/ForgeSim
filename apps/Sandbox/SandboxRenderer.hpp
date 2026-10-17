#pragma once

#include "SandboxObject.hpp"
#include "SceneLight.hpp"
#include "ShaderFileMonitor.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ForgeSim/Assets/MeshData.hpp>
#include <ForgeSim/Assets/SceneAsset.hpp>
#include <ForgeSim/Renderer/DebugGeometry.hpp>
#include <ForgeSim/Renderer/Bounds.hpp>
#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/DebugLineRenderer.hpp>
#include <ForgeSim/Renderer/OpenGL/EnvironmentMapProcessor.hpp>
#include <ForgeSim/Renderer/OpenGL/GpuMesh.hpp>
#include <ForgeSim/Renderer/OpenGL/HdrRenderTarget.hpp>
#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
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

		std::optional<SceneLightId> selectedLightId;

		SelectionHighlightSettings selectionHighlight;
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
			std::optional<SandboxObjectId> selectedObjectId,
			const glm::mat4& view,
			const glm::mat4& projection,
			const glm::vec3& cameraPosition,
			const SceneLightingView& light,
			const SandboxRenderSettings& settings);

		void ReloadChangedShaders();

		// Temporary integration path while SandboxRenderer owns the shared object mesh.
		// Move bounds ownership to the mesh resource when objects support distinct meshes.
		[[nodiscard]]
		const Renderer::AxisAlignedBoundingBox&
			ObjectLocalBounds() const noexcept;

	private:

		void BindMaterial(
			std::optional<Assets::MaterialIndex> materialIndex);

		void AppendSceneNodeSubmissions(
			Assets::SceneNodeIndex nodeIndex,
			const glm::mat4& parentNodeTransform,
			const glm::mat4& objectTransform,
			bool isSelected);

		struct RenderSubmission
		{
			const Renderer::OpenGL::GpuMesh* mesh = nullptr;
			std::optional<Assets::MaterialIndex> materialIndex;
			glm::mat4 modelTransform{ 1.0f };
			bool isSelected = false;
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
		};

		std::vector<RenderSubmission> m_RenderSubmissions;

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

		ShaderFileMonitor m_ToneMappingShaderMonitor;

		Renderer::OpenGL::ToneMappingRenderer
			m_ToneMappingRenderer;
	};
}