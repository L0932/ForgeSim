#pragma once

#include <cstdint>
#include <filesystem>

namespace ForgeSim::Sandbox
{
	class ShaderFileMonitor final
	{
	public:
		ShaderFileMonitor(
			std::filesystem::path vertexShaderPath,
			std::filesystem::path fragmentShaderPath);

		// Records the newly observed state before returning true.
		// This prevents a failed shader state from being retried every frame.
		[[nodiscard]] bool PollForChanges();

		[[nodiscard]]
		const std::filesystem::path&
			VertexShaderPath() const noexcept;

		[[nodiscard]]
		const std::filesystem::path&
			FragmentShaderPath() const noexcept;

	private:
		struct FileState
		{
			bool exists = false;

			std::filesystem::file_time_type
				lastWriteTime{};

			std::uintmax_t size = 0;

			[[nodiscard]]
			bool operator==(
				const FileState&) const noexcept = default;
		};

		[[nodiscard]] static FileState ReadState(
			const std::filesystem::path& path) noexcept;

		std::filesystem::path m_VertexShaderPath;
		std::filesystem::path m_FragmentShaderPath;

		FileState m_VertexShaderState;
		FileState m_FragmentShaderState;
	};
}