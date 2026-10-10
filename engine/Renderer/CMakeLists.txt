add_library(ForgeSimRenderer STATIC)

add_library(
	ForgeSim::Renderer
	ALIAS
		ForgeSimRenderer
)

target_sources(
	ForgeSimRenderer
	PRIVATE
		src/PerspectiveCamera.cpp
		src/Picking.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/ForgeSim/Renderer/PerspectiveCamera.hpp
			include/ForgeSim/Renderer/Picking.hpp
)

target_link_libraries(
	ForgeSimRenderer
	PUBLIC
		glm::glm-header-only
)

target_compile_features(
	ForgeSimRenderer
	PUBLIC
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimRenderer)

set_target_properties(
	ForgeSimRenderer
	PROPERTIES
		FOLDER "ForgeSim/Renderer"
)

add_subdirectory(OpenGL)