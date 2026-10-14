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
			std::string_view irradianceFragmentShaderSource);

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

	private:
		ShaderProgram m_ConversionShaderProgram;
		ShaderProgram m_IrradianceShaderProgram;

		Buffer m_CubeVertexBuffer;
		Buffer m_CubeIndexBuffer;
		VertexArray m_CubeVertexArray;

		Sampler m_EquirectangularSampler;

		unsigned int m_Framebuffer = 0;
		unsigned int m_DepthRenderbuffer = 0;

		void CaptureCubemapFaces(
			TextureCube& destination,
			std::uint32_t faceSize,
			ShaderProgram& shaderProgram);
	};
}