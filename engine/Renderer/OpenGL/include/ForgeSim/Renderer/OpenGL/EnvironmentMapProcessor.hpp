#pragma once

#include <cstdint>
#include <string_view>

#include <ForgeSim/Renderer/OpenGL/Buffer.hpp>
#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>
#include <ForgeSim/Renderer/OpenGL/ShaderProgram.hpp>
#include <ForgeSim/Renderer/OpenGL/TextureCube.hpp>
#include <ForgeSim/Renderer/OpenGL/VertexArray.hpp>

namespace ForgeSim::Renderer::OpenGL
{
	class Texture2D;

	class EnvironmentMapProcessor final
	{
	public:
		EnvironmentMapProcessor(
			std::string_view captureVertexShaderSource,
			std::string_view conversionFragmentShaderSource,
			std::string_view irradianceFragmentShaderSource,
			std::string_view specularPrefilterFragmentShaderSource,
			std::string_view fullscreenVertexShaderSource,
			std::string_view brdfIntegrationFragmentShaderSource);

		~EnvironmentMapProcessor();

		EnvironmentMapProcessor(
			const EnvironmentMapProcessor&) = delete;

		EnvironmentMapProcessor& operator=(
			const EnvironmentMapProcessor&) = delete;

		EnvironmentMapProcessor(
			EnvironmentMapProcessor&&) = delete;

		EnvironmentMapProcessor& operator=(
			EnvironmentMapProcessor&&) = delete;

		[[nodiscard]] TextureCube
			ConvertEquirectangularToCubemap(
				const Texture2D& equirectangularTexture,
				std::uint32_t cubemapSize);

		[[nodiscard]] TextureCube CreateDiffuseIrradianceMap(
			const TextureCube& environmentCubemap,
			std::uint32_t irradianceMapSize);

		[[nodiscard]] TextureCube CreatePrefilteredSpecularMap(
			const TextureCube& environmentCubemap,
			std::uint32_t specularMapSize,
			std::uint32_t mipLevelCount);

		[[nodiscard]] Texture2D CreateBrdfLookupTexture(
			std::uint32_t lookupSize);

	private:
		ShaderProgram m_ConversionShaderProgram;
		ShaderProgram m_IrradianceShaderProgram;
		ShaderProgram m_SpecularPrefilterShaderProgram;
		ShaderProgram m_BrdfIntegrationShaderProgram;

		Buffer m_CubeVertexBuffer;
		Buffer m_CubeIndexBuffer;
		VertexArray m_CubeVertexArray;

		// OpenGL core profile still requires a vertex array to be 
		// bound even when the shader generates vertices itself
		VertexArray m_FullscreenVertexArray;

		Sampler m_EquirectangularSampler;

		unsigned int m_Framebuffer = 0;
		unsigned int m_DepthRenderbuffer = 0;

		void CaptureCubemapFaces(
			TextureCube& destination,
			std::uint32_t faceSize,
			std::uint32_t mipLevel,
			ShaderProgram& shaderProgram);
	};
}