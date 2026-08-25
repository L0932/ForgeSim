add_executable(
	ForgeSimSandbox 
)

target_sources(
	ForgeSimSandbox
	PRIVATE
		Main.cpp
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
		ForgeSim::Glad
)

forgesim_set_project_warnings(ForgeSimSandbox)