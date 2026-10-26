if(NOT CPACK_BUILD_CONFIG MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
    message(
        FATAL_ERROR
        "Sandbox packages require a Release, RelWithDebInfo, "
        "or MinSizeRel configuration. "
        "Build that configuration first, then run cpack -C Release."
    )
endif()

if(CPACK_GENERATOR STREQUAL "NSIS")
    # Keep the installation location and registry identity stable
    # across releases so newer installers can find older versions.
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ForgeSim")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ForgeSim")

    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_DISPLAY_NAME "ForgeSim Sandbox")
    set(
        CPACK_NSIS_PACKAGE_NAME
        "ForgeSim Sandbox ${CPACK_PACKAGE_VERSION}"
    )

    # Distinguish the installer from the portable archive.
    set(
        CPACK_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_FILE_NAME}-Setup"
    )

    # Install the runtime as one application.
    set(CPACK_NSIS_COMPONENT_INSTALL OFF)

    # ForgeSimSandbox.exe is installed at the installation root,
    # rather than in a bin directory.
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(
        CPACK_PACKAGE_EXECUTABLES
        "ForgeSimSandbox" "ForgeSim Sandbox"
    )

    set(
        CPACK_NSIS_INSTALLED_ICON_NAME
        "ForgeSimSandbox.exe"
    )

    # Offer to remove an existing installation before installing
    # the replacement version.
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    # Launch through shortcuts without modifying the system PATH.
    set(CPACK_NSIS_MODIFY_PATH OFF)

    set(
        CPACK_NSIS_URL_INFO_ABOUT
        "https://github.com/L0932/ForgeSim"
    )

    set(
        CPACK_NSIS_HELP_LINK
        "https://github.com/L0932/ForgeSim/issues"
    )

    # Distribution licensing is still being decided. Do not show
    # CPack's default license text as ForgeSim's license.
    # Bundled third-party notices remain included.
    set(CPACK_NSIS_IGNORE_LICENSE_PAGE ON)

    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
endif()