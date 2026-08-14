cmake_minimum_required(VERSION 3.24)

include (FetchContent)

# set project name
project(
    ForgeSim
    VERSION 0.1.0
    DESCRIPTION "A modular real-time simulation framework and editor"
    LANGUAGES CXX
)

# Prevent accidental in-source builds.
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
message(
    FATAL_ERROR
    "In-source builds are not supported. "
    "Configure ForgeSim using a separate build directory."
)
endif()

# ---------------------------------------------------------------------------
# Project options
# ---------------------------------------------------------------------------

option(FORGESIM_BUILD_SANDBOX "Build the ForgeSim sandbox application" ON)
option(FORGESIM_BUILD_TESTS "Build ForgeSim tests" ON)
option(FORGESIM_WARNING_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---------------------------------------------------------------------------
# Global project configuration
# ---------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Allow project CMake modules to be included by name.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(CompilerWarnings)

# ---------------------------------------------------------------------------
# Engine modules
# ---------------------------------------------------------------------------

add_subdirectory(engine/core)
add_subdirectory(engine/Platform/GLFW)

# ---------------------------------------------------------------------------
# Applications
# ---------------------------------------------------------------------------

if(FORGESIM_BUILD_SANDBOX)
    add_subdirectory(apps/Sandbox)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if (FORGESIM_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()