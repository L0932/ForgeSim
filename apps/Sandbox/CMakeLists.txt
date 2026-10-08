add_executable(
	ForgeSimSandbox 
)

target_sources(
	ForgeSimSandbox
	PRIVATE
		Main.cpp
		SandboxApplication.hpp
		SandboxApplication.cpp
		FreeCameraController.hpp
		FreeCameraController.cpp
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