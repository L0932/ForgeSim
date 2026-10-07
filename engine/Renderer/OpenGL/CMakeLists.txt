
add_library(ForgeSimRendererOpenGL STATIC)

add_library(
	ForgeSim::RendererOpenGL
	ALIAS
	ForgeSimRendererOpenGL
)

target_sources(
		ForgeSimRendererOpenGL
	PRIVATE
		src/Buffer.cpp
		src/ShaderProgram.cpp
		src/VertexArray.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/ForgeSim/Renderer/OpenGL/Buffer.hpp 
			include/ForgeSim/Renderer/OpenGL/ShaderProgram.hpp
			include/ForgeSim/Renderer/OpenGL/VertexArray.hpp
)

target_include_directories(
	ForgeSimRendererOpenGL
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
	ForgeSimRendererOpenGL
	PRIVATE
		ForgeSim::Glad
)

target_compile_features(
		ForgeSimRendererOpenGL
	PUBLIC
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimRendererOpenGL)

set_target_properties(
	ForgeSimRendererOpenGL
	PROPERTIES
		FOLDER "ForgeSim/Renderer"
)