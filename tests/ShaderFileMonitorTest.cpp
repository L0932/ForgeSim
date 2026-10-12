#include "ShaderFileMonitor.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace
{
	class TemporaryDirectory final
	{
	public:
		TemporaryDirectory()
		{
			const auto identifier =
				std::chrono::steady_clock::now()
				.time_since_epoch()
				.count();

			m_Path =
				std::filesystem::temp_directory_path() /
				("ForgeSimShaderMonitorTest-" +
					std::to_string(identifier));

			std::filesystem::create_directories(m_Path);
		}

		~TemporaryDirectory()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Path, error);
		}

		[[nodiscard]]
		const std::filesystem::path& Path() const noexcept
		{
			return m_Path;
		}

	private:
		std::filesystem::path m_Path;
	};

	void WriteTextFile(
		const std::filesystem::path& path,
		std::string_view contents)
	{
		std::ofstream file{
			path,
			std::ios::out | std::ios::binary
		};

		if (!file)
		{
			throw std::runtime_error(
				"Could not create test file.");
		}

		file << contents;

		if (!file)
		{
			throw std::runtime_error(
				"Could not write test file.");
		}
	}
}

int main()
{
	using ForgeSim::Sandbox::ShaderFileMonitor;

	try
	{
		TemporaryDirectory temporaryDirectory;

		const auto vertexPath =
			temporaryDirectory.Path() / "Test.vert";

		const auto fragmentPath =
			temporaryDirectory.Path() / "Test.frag";

		WriteTextFile(
			vertexPath,
			"#version 460 core\nvoid main() {}\n");

		WriteTextFile(
			fragmentPath,
			"#version 460 core\nvoid main() {}\n");

		ShaderFileMonitor monitor{
			vertexPath,
			fragmentPath
		};

		if (monitor.PollForChanges())
		{
			std::cerr
				<< "New monitor incorrectly reported a change\n";

			return EXIT_FAILURE;
		}

		const auto originalVertexWriteTime =
			std::filesystem::last_write_time(vertexPath);

		std::filesystem::last_write_time(
			vertexPath,
			originalVertexWriteTime +
			std::chrono::seconds{ 1 });

		if (!monitor.PollForChanges())
		{
			std::cerr
				<< "Vertex shader change was not detected\n";

			return EXIT_FAILURE;
		}

		// The newly observed state should have been recorded.
		if (monitor.PollForChanges())
		{
			std::cerr
				<< "Unchanged state was reported repeatedly\n";

			return EXIT_FAILURE;
		}

		std::filesystem::remove(fragmentPath);

		if (!monitor.PollForChanges())
		{
			std::cerr
				<< "Removed fragment shader was not detected\n";

			return EXIT_FAILURE;
		}

		if (monitor.PollForChanges())
		{
			std::cerr
				<< "Missing file state was reported repeatedly\n";

			return EXIT_FAILURE;
		}

		WriteTextFile(
			fragmentPath,
			"#version 460 core\nvoid main() {}\n");

		if (!monitor.PollForChanges())
		{
			std::cerr
				<< "Restored fragment shader was not detected\n";

			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}