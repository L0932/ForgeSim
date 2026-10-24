set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_NAME "ForgeSim")
set(CPACK_PACKAGE_VENDOR "ForgeSim")
set(
    CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "ForgeSim real-time rendering and simulation Sandbox"
)

set(CPACK_PACKAGE_VERSION "${FORGESIM_VERSION_STRING}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(
    CPACK_PACKAGE_FILE_NAME
    "ForgeSim-${FORGESIM_VERSION_STRING}-Windows-${CMAKE_SYSTEM_PROCESSOR}"
)

set(
    CPACK_PACKAGE_DIRECTORY
    "${PROJECT_BINARY_DIR}/packages"
)

set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)

# Package only the application component, excluding dependency
# development files and unrelated installation rules.
set(
    CPACK_INSTALL_CMAKE_PROJECTS
    "${PROJECT_BINARY_DIR};${PROJECT_NAME};SandboxRuntime;/"
)

set(
    CPACK_PROJECT_CONFIG_FILE
    "${PROJECT_SOURCE_DIR}/cmake/PackageSandboxOptions.cmake"
)

include(CPack)