#pragma once

#include <filesystem>

namespace ForgeSim::Platform
{
    // Returns the absolute directory containing the running executable.
    // Throws if the platform cannot determine that directory.
    [[nodiscard]] std::filesystem::path ExecutableDirectory();

    // Returns the operating system's per-user application-data directory.
    // Does not create directories.
    [[nodiscard]] std::filesystem::path UserDataDirectory();
}