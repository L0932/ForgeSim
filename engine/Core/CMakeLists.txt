add_library(ForgeSimCore STATIC)

add_library(ForgeSim::Core ALIAS ForgeSimCore)

target_sources(
    ForgeSimCore
    PRIVATE
        src/Application.cpp
    PUBLIC
        FILE_SET 
            HEADERS
        BASE_DIRS 
            include
        FILES
            include/ForgeSim/Core/Application.hpp
)

target_include_directories(
    ForgeSimCore
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_compile_features(
    ForgeSimCore 
    PUBLIC 
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimCore)

set_target_properties(
    ForgeSimCore
    PROPERTIES
        FOLDER "ForgeSim/Engine"
)