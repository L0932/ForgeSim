if(NOT CPACK_BUILD_CONFIG MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
    message(
        FATAL_ERROR
        "Sandbox packages require a Release, RelWithDebInfo, "
        "or MinSizeRel configuration. "
        "Build that configuration first, then run cpack -C Release."
    )
endif()