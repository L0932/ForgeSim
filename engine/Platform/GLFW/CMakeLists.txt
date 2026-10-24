add_library(ForgeSimPlatformGLFW STATIC)

add_library(
    ForgeSim::PlatformGLFW 
    ALIAS 
    ForgeSimPlatformGLFW
)

target_sources(
    ForgeSimPlatformGLFW
    PRIVATE
        src/ApplicationPaths.cpp
        src/FileDialogs.cpp
        src/GlfwWindow.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/ForgeSim/Platform/ApplicationPaths.hpp
            include/ForgeSim/Platform/FileDialogs.hpp
            include/ForgeSim/Platform/GlfwWindow.hpp
)

target_link_libraries(
    ForgeSimPlatformGLFW
    PUBLIC
        ForgeSim::Platform
    PRIVATE
        ForgeSim::Core
        ForgeSim::Glad
        glfw
)

if(WIN32)
    target_link_libraries(
        ForgeSimPlatformGLFW
        PRIVATE
            ole32
            shell32
            uuid
    )
endif()

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