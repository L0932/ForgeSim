cmake_minimum_required(VERSION 3.24)

# set project name
project(
    ForgeSim
    VERSION 0.1.0
    DESCRIPTION "A modular real-time simulation framework and editor"
    LANGUAGES C CXX
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

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

include (FetchContent)

# GLFW 

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

# GLAD

add_subdirectory(external/glad)

# GLM

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm)

# cgltf

FetchContent_Declare(
    cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG v1.15
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(cgltf)

add_library(ForgeSimCgltf INTERFACE)

add_library(
    ForgeSim::Cgltf
    ALIAS
        ForgeSimCgltf
)

target_include_directories(
    ForgeSimCgltf
    SYSTEM
    INTERFACE
        "${cgltf_SOURCE_DIR}"
)

# ---------------------------------------------------------------------------
# Engine modules
# ---------------------------------------------------------------------------

add_subdirectory(engine/core)
add_subdirectory(engine/Assets)
add_subdirectory(engine/Platform)
add_subdirectory(engine/Renderer)

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
    enable_testing()
    add_subdirectory(tests)
endif()