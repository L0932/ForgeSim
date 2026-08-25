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

#include <iosteam>

target_link_libraries(
    ForgeSimPlatformGLFW
    PRIVATE
        ForgeSim::Core
        ForgeSim::Glad
        glfw
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