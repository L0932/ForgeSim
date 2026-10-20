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
	    GLM_ENABLE_EXPERIMENTAL
		FORGESIM_SANDBOX_ASSET_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/assets"
		FORGESIM_SANDBOX_SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
)

target_sources(
	ForgeSimSandbox
	PRIVATE
		Main.cpp
		LightGizmoTransform.hpp
		LightGizmoTransform.cpp
		SandboxApplication.hpp
		SandboxApplication.cpp
		SandboxDebugUI.hpp
		SandboxDebugUI.cpp
		SandboxObject.hpp
		SandboxRenderer.hpp
		SandboxRenderer.cpp
		SandboxScene.hpp
		SandboxScene.cpp
		SceneEditHistory.hpp
		SceneEditHistory.cpp
		SceneTransformEditor.hpp
		SceneTransformEditor.cpp
		ShaderFileMonitor.hpp
		ShaderFileMonitor.cpp
		FreeCameraController.hpp
		FreeCameraController.cpp
		GizmoTransform.hpp
		GizmoTransform.cpp
		LightSubmission.hpp
		LightSubmission.cpp
		Transform.hpp
		Transform.cpp
		ViewportSelectionController.hpp
		ViewportSelectionController.cpp
		ViewportTransformGizmo.hpp
		ViewportTransformGizmo.cpp
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
		ForgeSim::ImGui
		ForgeSim::ImGuizmo
		glm::glm-header-only
)

forgesim_set_project_warnings(ForgeSimSandbox)