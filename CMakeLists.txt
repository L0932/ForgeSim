cmake_minimum_required(VERSION 3.24)

# set project name
project(
    ForgeSim
    VERSION 0.1.0
    DESCRIPTION "A modular real-time simulation framework and editor"
    LANGUAGES C CXX
)

set(
    FORGESIM_VERSION_PRERELEASE
    "dev"
    CACHE STRING
    "Prerelease identifier; empty for a stable release"
)

if(NOT FORGESIM_VERSION_PRERELEASE STREQUAL "")
    if(NOT FORGESIM_VERSION_PRERELEASE MATCHES
        "^[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*$")
        message(
            FATAL_ERROR
            "FORGESIM_VERSION_PRERELEASE must contain "
            "dot-separated letters, digits, or hyphens."
        )
    endif()
endif()

set(FORGESIM_VERSION_STRING "${PROJECT_VERSION}")

if(NOT FORGESIM_VERSION_PRERELEASE STREQUAL "")
    string(
        APPEND FORGESIM_VERSION_STRING
        "-${FORGESIM_VERSION_PRERELEASE}"
    )
endif()

message(STATUS "ForgeSim version: ${FORGESIM_VERSION_STRING}")

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

# Dear ImGui

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.92.9b-docking
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)

add_library(ForgeSimImGui STATIC)

add_library(
    ForgeSim::ImGui
    ALIAS
        ForgeSimImGui
)

target_sources(
    ForgeSimImGui
    PRIVATE
    	"${imgui_SOURCE_DIR}/imgui.cpp"
		"${imgui_SOURCE_DIR}/imgui_draw.cpp"
		"${imgui_SOURCE_DIR}/imgui_tables.cpp"
		"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
		"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(
    ForgeSimImGui
    SYSTEM
    PUBLIC
    	"${imgui_SOURCE_DIR}"
		"${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(
	ForgeSimImGui
	PRIVATE
		glfw
)

target_compile_features(
	ForgeSimImGui
	PRIVATE
		cxx_std_20
)

target_compile_definitions(
	ForgeSimImGui
	PRIVATE
		GLFW_INCLUDE_NONE
)

set_target_properties(
	ForgeSimImGui
	PROPERTIES
		FOLDER "ThirdParty"
)

# ImGuizmo

if(FORGESIM_BUILD_SANDBOX)
    FetchContent_Declare(
        imguizmo
        GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
        GIT_TAG 105e4fde45e4e74aae8973d98750820ef7aca7e2

        # Populate sources without running upstream's CMake project.
        SOURCE_SUBDIR forgesim-source-only
    )

    FetchContent_MakeAvailable(imguizmo)

    add_library(ForgeSimImGuizmo STATIC)

    add_library(
        ForgeSim::ImGuizmo
        ALIAS
            ForgeSimImGuizmo
    )

    target_sources(
        ForgeSimImGuizmo
        PRIVATE
            "${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp"
    )

    target_include_directories(
        ForgeSimImGuizmo
        SYSTEM
        PUBLIC
            "${imguizmo_SOURCE_DIR}/src"
    )

    target_link_libraries(
        ForgeSimImGuizmo
        PUBLIC
            ForgeSim::ImGui
    )

    target_compile_features(
        ForgeSimImGuizmo
        PRIVATE
            cxx_std_20
    )

    set_target_properties(
        ForgeSimImGuizmo
        PROPERTIES
            FOLDER "ThirdParty"
    )
endif()

# stb_image

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb.git
	GIT_TAG 2c980bb59875b0d32144a71867fbdebb2f77cd20
)

FetchContent_MakeAvailable(stb)

add_library(ForgeSimStb INTERFACE)

target_include_directories(
	ForgeSimStb
	SYSTEM INTERFACE
		${stb_SOURCE_DIR}
)

# JSON

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE
)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(nlohmann_json)

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

# ---------------------------------------------------------------------------
# Packaging
# ---------------------------------------------------------------------------

if(FORGESIM_BUILD_SANDBOX AND WIN32)
    include("${PROJECT_SOURCE_DIR}/cmake/PackageSandbox.cmake")
endif()