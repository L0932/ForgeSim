#include <ForgeSim/Platform/ApplicationPaths.hpp>

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <vector>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <ShlObj.h>

#elif defined(__linux__)

#include <cerrno>
#include <unistd.h>

#endif

namespace ForgeSim::Platform
{
    std::filesystem::path ExecutableDirectory()
    {
#if defined(_WIN32)

        std::vector<wchar_t> buffer(512);

        for (;;)
        {
            const DWORD length = GetModuleFileNameW(
                nullptr,
                buffer.data(),
                static_cast<DWORD>(buffer.size()));

            if (length == 0)
            {
                throw std::system_error(
                    static_cast<int>(GetLastError()),
                    std::system_category(),
                    "Cannot determine the executable path");
            }

            if (length < buffer.size())
            {
                return std::filesystem::path{
                    buffer.data(),
                    buffer.data() + length
                }.parent_path();
            }

            if (buffer.size() >= 32768)
            {
                throw std::runtime_error(
                    "The executable path exceeds the supported length.");
            }

            buffer.resize(buffer.size() * 2);
        }

#elif defined(__linux__)

        std::vector<char> buffer(512);

        for (;;)
        {
            const ssize_t length = readlink(
                "/proc/self/exe",
                buffer.data(),
                buffer.size());

            if (length < 0)
            {
                throw std::system_error(
                    errno,
                    std::generic_category(),
                    "Cannot determine the executable path");
            }

            const auto size = static_cast<std::size_t>(length);

            if (size < buffer.size())
            {
                return std::filesystem::path{
                    buffer.data(),
                    buffer.data() + size
                }.parent_path();
            }

            buffer.resize(buffer.size() * 2);
        }

#else

        throw std::runtime_error(
            "Executable-directory lookup is not implemented "
            "for this platform.");

#endif
    }

    std::filesystem::path UserDataDirectory()
    {
#if defined(_WIN32)

        PWSTR rawPath = nullptr;

        const HRESULT result = SHGetKnownFolderPath(
            FOLDERID_LocalAppData,
            0,
            nullptr,
            &rawPath);

        const auto release = [](wchar_t* value)
            {
                CoTaskMemFree(value);
            };

        const std::unique_ptr<wchar_t, decltype(release)> owner{
            rawPath,
            release
        };

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Cannot locate the user's application-data directory.");
        }

        return std::filesystem::path{ owner.get() };

#elif defined(__linux__)

        if (const char* value = std::getenv("XDG_DATA_HOME");
            value != nullptr && value[0] != '\0')
        {
            const std::filesystem::path directory{ value };

            if (directory.is_absolute())
            {
                return directory;
            }
        }

        if (const char* value = std::getenv("HOME");
            value != nullptr && value[0] != '\0')
        {
            const std::filesystem::path directory{ value };

            if (directory.is_absolute())
            {
                return directory / ".local" / "share";
            }
        }

        throw std::runtime_error(
            "Cannot locate the user's application-data directory.");

#else

        throw std::runtime_error(
            "User-data directory lookup is not implemented "
            "for this platform.");

#endif
    }
}