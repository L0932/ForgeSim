#include <ForgeSim/Renderer/OpenGL/Sampler.hpp>

#include <stdexcept>
#include <utility>

#include <glad/gl.h>

namespace ForgeSim::Renderer::OpenGL
{
	namespace
	{
		[[nodiscard]] GLint ToOpenGL(
			MinificationFilter filter)
		{
			switch (filter)
			{
			case MinificationFilter::Nearest:
				return GL_NEAREST;

			case MinificationFilter::Linear:
				return GL_LINEAR;

			case MinificationFilter::NearestMipmapNearest:
				return GL_NEAREST_MIPMAP_NEAREST;

			case MinificationFilter::LinearMipmapNearest:
				return GL_LINEAR_MIPMAP_NEAREST;

			case MinificationFilter::NearestMipmapLinear:
				return GL_NEAREST_MIPMAP_LINEAR;

			case MinificationFilter::LinearMipmapLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			}

			throw std::logic_error(
				"Unsupported texture minification filter.");
		}

		[[nodiscard]] GLint ToOpenGL(
			MagnificationFilter filter)
		{
			switch (filter)
			{
			case MagnificationFilter::Nearest:
				return GL_NEAREST;

			case MagnificationFilter::Linear:
				return GL_LINEAR;
			}

			throw std::logic_error(
				"Unsupported texture magnification filter.");
		}

		[[nodiscard]] GLint ToOpenGL(
			SamplerWrap wrap)
		{
			switch (wrap)
			{
			case SamplerWrap::Repeat:
				return GL_REPEAT;

			case SamplerWrap::MirroredRepeat:
				return GL_MIRRORED_REPEAT;

			case SamplerWrap::ClampToEdge:
				return GL_CLAMP_TO_EDGE;
			}

			throw std::logic_error(
				"Unsupported texture wrapping mode.");
		}
	}

	Sampler::Sampler(
		const SamplerSpecification& specification)
	{
		glCreateSamplers(
			1,
			&m_Handle);

		if (m_Handle == 0)
		{
			throw std::runtime_error(
				"Failed to create OpenGL sampler.");
		}

		glSamplerParameteri(
			m_Handle,
			GL_TEXTURE_MIN_FILTER,
			ToOpenGL(
				specification.minificationFilter));

		glSamplerParameteri(
			m_Handle,
			GL_TEXTURE_MAG_FILTER,
			ToOpenGL(
				specification.magnificationFilter));

		glSamplerParameteri(
			m_Handle,
			GL_TEXTURE_WRAP_S,
			ToOpenGL(
				specification.wrapU));

		glSamplerParameteri(
			m_Handle,
			GL_TEXTURE_WRAP_T,
			ToOpenGL(
				specification.wrapV));
	}

	Sampler::~Sampler()
	{
		if (m_Handle != 0)
		{
			glDeleteSamplers(
				1,
				&m_Handle);
		}
	}

	Sampler::Sampler(
		Sampler&& other) noexcept
		: m_Handle(
			std::exchange(
				other.m_Handle,
				0))
	{}

	Sampler& Sampler::operator=(
		Sampler&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		if (m_Handle != 0)
		{
			glDeleteSamplers(
				1,
				&m_Handle);
		}

		m_Handle =
			std::exchange(
				other.m_Handle,
				0);

		return *this;
	}

	void Sampler::Bind(
		std::uint32_t textureUnit) const noexcept
	{
		glBindSampler(
			textureUnit,
			m_Handle);
	}
}