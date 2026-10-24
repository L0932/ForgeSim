# Install only the application runtime, not engine development libraries.
install(
    TARGETS ForgeSimSandbox
    RUNTIME
        DESTINATION .
        COMPONENT SandboxRuntime
        CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

# Install shaders from source rather than potentially stale build output.
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/shaders/"
    DESTINATION assets/shaders
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
    FILES_MATCHING
        PATTERN "*.vert"
        PATTERN "*.frag"
)

# Explicitly select bundled models. Do not package arbitrary local assets.
install(
    FILES
        "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/models/PbrMaterialGallery.glb"
        "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/models/BoxVertexColors.glb"
        "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/models/README.md"
    DESTINATION assets/models
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

install(
    FILES
        "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/environments/studio_small_01_4k.hdr"
        "${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/environments/README.md"
    DESTINATION assets/environments
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

# Collect redistributable compiler runtime libraries for MSVC builds.
if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES FALSE)

    include(InstallRequiredSystemLibraries)

    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install(
            PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            DESTINATION .
            COMPONENT SandboxRuntime
            CONFIGURATIONS Release RelWithDebInfo MinSizeRel
        )
    endif()
endif()

install(
    FILES "${PROJECT_SOURCE_DIR}/docs/InstalledApplication.md"
    DESTINATION .
    RENAME README.md
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

install(
    FILES "${PROJECT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
    DESTINATION .
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

function(forgesim_install_notice dependency source_file)
    if(NOT EXISTS "${source_file}")
        message(
            FATAL_ERROR
            "Missing ${dependency} notice: ${source_file}"
        )
    endif()

    install(
        FILES "${source_file}"
        DESTINATION "notices/${dependency}"
        COMPONENT SandboxRuntime
        CONFIGURATIONS Release RelWithDebInfo MinSizeRel
    )
endfunction()

forgesim_install_notice(
    glfw "${glfw_SOURCE_DIR}/LICENSE.md"
)

forgesim_install_notice(
    glm "${glm_SOURCE_DIR}/copying.txt"
)

forgesim_install_notice(
    cgltf "${cgltf_SOURCE_DIR}/LICENSE"
)

forgesim_install_notice(
    imgui "${imgui_SOURCE_DIR}/LICENSE.txt"
)

forgesim_install_notice(
    imguizmo "${imguizmo_SOURCE_DIR}/LICENSE"
)

forgesim_install_notice(
    stb "${stb_SOURCE_DIR}/LICENSE"
)

forgesim_install_notice(
    nlohmann_json "${nlohmann_json_SOURCE_DIR}/LICENSE.MIT"
)

forgesim_install_notice(
    glad "${PROJECT_SOURCE_DIR}/external/glad/LICENSE"
)

forgesim_install_notice(
    glad "${PROJECT_SOURCE_DIR}/external/glad/Apache-2.0.txt"
)

forgesim_install_notice(
    glad "${PROJECT_SOURCE_DIR}/external/glad/CC0-1.0.txt"
)

# Preserve the embedded Khronos platform-header notice.
forgesim_install_notice(
    glad "${PROJECT_SOURCE_DIR}/external/glad/include/KHR/khrplatform.h"
)

install(
    FILES "${PROJECT_SOURCE_DIR}/CHANGELOG.md"
    DESTINATION .
    COMPONENT SandboxRuntime
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

set(
    FORGESIM_RELEASE_NOTES
    "${PROJECT_SOURCE_DIR}/docs/releases/${FORGESIM_VERSION_STRING}.md"
)

if(EXISTS "${FORGESIM_RELEASE_NOTES}")
    install(
        FILES "${FORGESIM_RELEASE_NOTES}"
        DESTINATION .
        RENAME RELEASE_NOTES.md
        COMPONENT SandboxRuntime
        CONFIGURATIONS Release RelWithDebInfo MinSizeRel
    )
endif()