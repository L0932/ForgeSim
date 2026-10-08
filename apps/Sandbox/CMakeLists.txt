add_executable(
	ForgeSimSandbox 
)

add_custom_command(
	TARGET ForgeSimSandbox
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:ForgeSimSandbox>/assets"
	COMMENT
		"Copying Sandbox assets"
	VERBATIM
)

target_compile_definitions(
	ForgeSimSandbox
	PRIVATE
		FORGESIM_SANDBOX_ASSET_DIRECTORY="$<TARGET_FILE_DIR:ForgeSimSandbox>/assets"
)

target_sources(
	ForgeSimSandbox
	PRIVATE
		Main.cpp
		SandboxApplication.hpp
		SandboxApplication.cpp
		FreeCameraController.hpp
		FreeCameraController.cpp
		SandboxObject.hpp
		Transform.hpp
		Transform.cpp
)

target_compile_features(
	ForgeSimSandbox 
	PRIVATE
		cxx_std_20
)

target_link_libraries(
	ForgeSimSandbox
	PRIVATE
		ForgeSim::Core
		ForgeSim::PlatformGLFW
		ForgeSim::Renderer
		ForgeSim::RendererOpenGL
		ForgeSim::Glad
		glm::glm-header-only
)

forgesim_set_project_warnings(ForgeSimSandbox)