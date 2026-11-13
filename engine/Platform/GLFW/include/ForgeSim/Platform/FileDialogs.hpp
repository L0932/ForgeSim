#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace ForgeSim::Platform
{
	class GlfwWindow;

	enum class FileDialogMode
	{
		Open,
        SelectFolder,
		Save
	};

    enum class FileDialogPurpose
    {
        General,
        ModelImport,
        ModelRecovery
    };

	struct FileDialogSpecification
	{
		std::filesystem::path defaultDirectory;
		std::filesystem::path suggestedFileName;

		std::wstring title;
		std::wstring filterName = L"All files";
		std::wstring filterPattern = L"*.*";
		std::wstring defaultExtension;
        // Stable purpose-specific Windows history, independent of the title/filter.
        FileDialogPurpose purpose = FileDialogPurpose::General;
	};

	[[nodiscard]] std::optional<std::filesystem::path>
		ShowFileDialog(
			GlfwWindow& owner,
			FileDialogMode mode,
			const FileDialogSpecification& specification);
}