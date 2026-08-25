add_executable(
	ForgeSimSandbox 
	Main.cpp
)

target_link_libraries(
	ForgeSimSandbox
	PRIVATE
	ForgeSim::Core
	ForgeSim::PlatformGLFW
)

forgesim_set_project_warnings(ForgeSimSandbox)