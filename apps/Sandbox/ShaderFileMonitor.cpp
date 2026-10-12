#include "ShaderFileMonitor.hpp"

#include <system_error>
#include <utility>

namespace ForgeSim::Sandbox
{
	ShaderFileMonitor::ShaderFileMonitor(
		std::filesystem::path vertexShaderPath,
		std::filesystem::path fragmentShaderPath)
		: m_VertexShaderPath(
			std::move(vertexShaderPath))
		, m_FragmentShaderPath(
			std::move(fragmentShaderPath))
		, m_VertexShaderState(
			ReadState(m_VertexShaderPath))
		, m_FragmentShaderState(
			ReadState(m_FragmentShaderPath))
	{}

	bool ShaderFileMonitor::PollForChanges()
	{
		const FileState currentVertexState =
			ReadState(m_VertexShaderPath);

		const FileState currentFragmentState =
			ReadState(m_FragmentShaderPath);

		const bool changed =
			currentVertexState != m_VertexShaderState ||
			currentFragmentState != m_FragmentShaderState;

		if (changed)
		{
			// Record the attempted file state even if compilation
			// later fails. Another attempt occurs only after a
			// subsequent file change.
			m_VertexShaderState = currentVertexState;
			m_FragmentShaderState = currentFragmentState;
		}

		return changed;
	}

	const std::filesystem::path&
		ShaderFileMonitor::VertexShaderPath()
		const noexcept
	{
		return m_VertexShaderPath;
	}

	const std::filesystem::path&
		ShaderFileMonitor::FragmentShaderPath()
		const noexcept
	{
		return m_FragmentShaderPath;
	}

	ShaderFileMonitor::FileState
		ShaderFileMonitor::ReadState(
			const std::filesystem::path& path) noexcept
	{
		std::error_code error;

		const bool exists =
			std::filesystem::is_regular_file(
				path,
				error);

		if (error || !exists)
		{
			return {};
		}

		const auto lastWriteTime =
			std::filesystem::last_write_time(
				path,
				error);

		if (error)
		{
			return {};
		}

		const std::uintmax_t size =
			std::filesystem::file_size(
				path,
				error);

		if (error)
		{
			return {};
		}

		return FileState{
			.exists = true,
			.lastWriteTime = lastWriteTime,
			.size = size
		};
	}
}