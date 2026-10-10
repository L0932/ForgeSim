add_executable(
	ForgeSimSandbox 
)

add_custom_target(
	ForgeSimSandboxAssets
	ALL
	COMMAND
		${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:ForgeSimSandbox>/assets"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/Grid.vert"
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/Grid.frag"
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/Object.vert"
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/Object.frag"
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/models/BoxVertexColors.glb"
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/models/README.md"
	COMMENT
		"Copying Sandbox assets"
	VERBATIM
)

add_dependencies(
	ForgeSimSandbox
	ForgeSimSandboxAssets
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
		ViewportSelectionController.hpp
		ViewportSelectionController.cpp
)

target_compile_features(
	ForgeSimSandbox 
	PRIVATE
		cxx_std_20
)

target_link_libraries(
	ForgeSimSandbox
	PRIVATE
		ForgeSim::Assets
		ForgeSim::Core
		ForgeSim::PlatformGLFW
		ForgeSim::Renderer
		ForgeSim::RendererOpenGL
		ForgeSim::Glad
		glm::glm-header-only
)

forgesim_set_project_warnings(ForgeSimSandbox)