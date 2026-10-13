#pragma once

#include <cstdint>

namespace ForgeSim::Renderer::OpenGL
{
	enum class MinificationFilter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	enum class MagnificationFilter
	{
		Nearest,
		Linear
	};

	enum class SamplerWrap
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge
	};

	struct SamplerSpecification
	{
		MinificationFilter minificationFilter =
			MinificationFilter::LinearMipmapLinear;

		MagnificationFilter magnificationFilter =
			MagnificationFilter::Linear;

		SamplerWrap wrapU = SamplerWrap::Repeat;
		SamplerWrap wrapV = SamplerWrap::Repeat;
	};

	class Sampler final
	{
	public:
		explicit Sampler(
			const SamplerSpecification& specification = {});

		~Sampler();

		Sampler(const Sampler&) = delete;

		Sampler& operator=(
			const Sampler&) = delete;

		Sampler(Sampler&& other) noexcept;

		Sampler& operator=(
			Sampler&& other) noexcept;

		void Bind(
			std::uint32_t textureUnit) const noexcept;

	private:
		unsigned int m_Handle = 0;
	};
}