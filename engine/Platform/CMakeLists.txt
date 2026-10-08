add_library(ForgeSimPlatform STATIC)

add_library(
	ForgeSim::Platform
	ALIAS
		ForgeSimPlatform
)

target_sources(
	ForgeSimPlatform
	PRIVATE
		src/InputState.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/ForgeSim/Platform/InputState.hpp
)

target_compile_features(
	ForgeSimPlatform
	PUBLIC
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimPlatform)

set_target_properties(
	ForgeSimPlatform
	PROPERTIES
		FOLDER "ForgeSim/Platform"
)

add_subdirectory(GLFW)