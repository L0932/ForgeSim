#pragma once

#include "SandboxObject.hpp"
#include "ShaderFileMonitor.hpp"

#include <cstddef>
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
#include <ForgeSim/Renderer/OpenGL/GpuMesh.hpp>
#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/Texture2D.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>

namespace ForgeSim::Sandbox
{
	struct DirectionalLight
	{
		// World-space direction from the surface toward the light.
		glm::vec3 directionToLight{
			-0.5f,
			1.0f,
			0.3f
		};

		glm::vec3 color{ 1.0f };
		float intensity = 3.0f;
	};

	class SandboxRenderer final
	{
	public:
		SandboxRenderer();

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
			const DirectionalLight& light,
			bool showGrid,
			bool showObjectBounds);

		void ReloadChangedShaders();

		// Temporary integration path while SandboxRenderer owns the shared object mesh.
		// Move bounds ownership to the mesh resource when objects support distinct meshes.
		[[nodiscard]]
		const Renderer::AxisAlignedBoundingBox&
			ObjectLocalBounds() const noexcept;

	private:

		void BindMaterial(
			std::optional<Assets::MaterialIndex> materialIndex);

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

			glm::vec4 baseColorFactor{ 1.0f };

			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			float normalScale = 1.0f;
		};

		std::vector<RenderSubmission> m_RenderSubmissions;

		Assets::SceneAsset m_ObjectSceneAsset;
		Renderer::AxisAlignedBoundingBox m_ObjectLocalBounds;

		ShaderFileMonitor m_ObjectShaderMonitor;
		Renderer::OpenGL::ShaderProgram m_ObjectShaderProgram;

		std::vector<GpuPrimitiveResource> 
			m_GpuPrimitiveResources;

		std::vector<Renderer::OpenGL::Texture2D>
			m_GpuTextures;

		std::vector<Renderer::OpenGL::Sampler>
			m_GpuSamplers;

		std::vector<GpuMaterialResource>
			m_GpuMaterialResources;

		Renderer::OpenGL::Texture2D m_FallbackWhiteTexture;
		Renderer::OpenGL::Texture2D m_FallbackNormalTexture;
		Renderer::OpenGL::Sampler m_DefaultSampler;

		std::vector<ForgeSim::Renderer::DebugLineVertex>
			m_GridVertices;

		std::vector<Renderer::DebugLineVertex>
			m_BoundsVertices;

		ShaderFileMonitor m_GridShaderMonitor;
		ForgeSim::Renderer::OpenGL::DebugLineRenderer
			m_DebugLineRenderer;
	};
}