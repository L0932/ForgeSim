#include <ForgeSim/Platform/FileDialogs.hpp>
#include <ForgeSim/Platform/GlfwWindow.hpp>

#include <stdexcept>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <system_error>

namespace ForgeSim::Platform
{
    namespace
    {
        void CheckResult(HRESULT result, const char* operation)
        {
            if (FAILED(result))
            {
                throw std::system_error(
                    static_cast<int>(result),
                    std::system_category(),
                    operation);
            }
        }

        class ComApartment final
        {
        public:
            ComApartment()
            {
                CheckResult(
                    CoInitializeEx(
                        nullptr,
                        COINIT_APARTMENTTHREADED |
                        COINIT_DISABLE_OLE1DDE),
                    "Initialize file dialog");
            }

            ~ComApartment()
            {
                CoUninitialize();
            }

            ComApartment(const ComApartment&) = delete;
            ComApartment& operator=(const ComApartment&) = delete;
        };

        // Keep these IDs stable: Windows associates dialog history with them.
        constexpr GUID openDialogId{
            0x2b4173d1, 0xa634, 0x461c,
            { 0x9a, 0x31, 0x29, 0x68, 0xf5, 0xc7, 0x28, 0x41 }
        };

        constexpr GUID saveDialogId{
            0xcb5e901f, 0x6832, 0x4455,
            { 0x8e, 0xd9, 0x71, 0x0d, 0x54, 0xb2, 0x16, 0x93 }
        };
    }

    std::optional<std::filesystem::path> ShowFileDialog(
        GlfwWindow& owner,
        FileDialogMode mode,
        const FileDialogSpecification& specification)
    {
        if (specification.defaultDirectory.empty())
        {
            throw std::invalid_argument(
                "File dialog requires a default directory.");
        }

        const std::filesystem::path defaultDirectory =
            std::filesystem::absolute(
                specification.defaultDirectory).lexically_normal();

        std::filesystem::create_directories(defaultDirectory);

        ComApartment apartment;

        Microsoft::WRL::ComPtr<IFileDialog> dialog;

        const bool saving = mode == FileDialogMode::Save;

        CheckResult(
            CoCreateInstance(
                saving ? CLSID_FileSaveDialog : CLSID_FileOpenDialog,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(dialog.GetAddressOf())),
            "Create file dialog");

        CheckResult(
            dialog->SetClientGuid(
                saving ? saveDialogId : openDialogId),
            "Set file dialog history");

        FILEOPENDIALOGOPTIONS options{};

        CheckResult(
            dialog->GetOptions(&options),
            "Read file dialog options");

        options |=
            FOS_FORCEFILESYSTEM |
            FOS_PATHMUSTEXIST |
            FOS_NOCHANGEDIR;

        options |= saving
            ? FOS_OVERWRITEPROMPT
            : FOS_FILEMUSTEXIST;

        CheckResult(
            dialog->SetOptions(options),
            "Configure file dialog");

        const COMDLG_FILTERSPEC filters[]{
            {
                specification.filterName.c_str(),
                specification.filterPattern.c_str()
            },
            { L"All files", L"*.*" }
        };

        CheckResult(
            dialog->SetFileTypes(2, filters),
            "Set file dialog filters");

        CheckResult(
            dialog->SetFileTypeIndex(1),
            "Select file dialog filter");

        if (!specification.title.empty())
        {
            CheckResult(
                dialog->SetTitle(specification.title.c_str()),
                "Set file dialog title");
        }

        if (!specification.defaultExtension.empty())
        {
            CheckResult(
                dialog->SetDefaultExtension(
                    specification.defaultExtension.c_str()),
                "Set file dialog extension");
        }

        if (saving && !specification.suggestedFileName.empty())
        {
            CheckResult(
                dialog->SetFileName(
                    specification.suggestedFileName
                    .filename().c_str()),
                "Set suggested filename");
        }

        Microsoft::WRL::ComPtr<IShellItem> defaultFolder;

        CheckResult(
            SHCreateItemFromParsingName(
                defaultDirectory.c_str(),
                nullptr,
                IID_PPV_ARGS(defaultFolder.GetAddressOf())),
            "Resolve default scene directory");

        // Use the default only when Windows has no remembered location.
        CheckResult(
            dialog->SetDefaultFolder(defaultFolder.Get()),
            "Set default scene directory");

        const HRESULT showResult =
            dialog->Show(glfwGetWin32Window(owner.NativeHandle()));

        if (showResult == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            return std::nullopt;
        }

        CheckResult(showResult, "Show file dialog");

        Microsoft::WRL::ComPtr<IShellItem> selectedItem;

        CheckResult(
            dialog->GetResult(selectedItem.GetAddressOf()),
            "Read selected file");

        PWSTR rawPath = nullptr;

        const HRESULT pathResult =
            selectedItem->GetDisplayName(
                SIGDN_FILESYSPATH,
                &rawPath);

        const std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)>
            selectedPath{ rawPath, &CoTaskMemFree };

        CheckResult(pathResult, "Read selected file path");

        return std::filesystem::path{ selectedPath.get() };
    }
}

#else

namespace ForgeSim::Platform
{
    std::optional<std::filesystem::path> ShowFileDialog(
        GlfwWindow&,
        FileDialogMode,
        const FileDialogSpecification&)
    {
        throw std::runtime_error(
            "Native file dialogs are not implemented on this platform.");
    }
}

#endif