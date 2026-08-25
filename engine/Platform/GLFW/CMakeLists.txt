add_library(ForgeSimPlatformGLFW STATIC)

add_library(
    ForgeSim::PlatformGLFW 
    ALIAS 
    ForgeSimPlatformGLFW
)

target_sources(
    ForgeSimPlatformGLFW
    PRIVATE
        src/GlfwWindow.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/ForgeSim/Platform/GlfwWindow.hpp
)

target_include_directories(
    ForgeSimPlatformGLFW
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_link_libraries(
    ForgeSimPlatformGLFW
    PRIVATE
        ForgeSim::Core
        glfw
        ForgeSimGlad
)

target_compile_features(
    ForgeSimPlatformGLFW 
    PUBLIC 
        cxx_std_20
)

target_compile_definitions(
    ForgeSimPlatformGLFW
    PRIVATE
        GLFW_INCLUDE_NONE
)

forgesim_set_project_warnings(ForgeSimPlatformGLFW)

set_target_properties(
    ForgeSimPlatformGLFW
    PROPERTIES
        FOLDER "ForgeSim/Platform"
)