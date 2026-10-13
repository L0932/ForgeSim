
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
		src/DebugLineRenderer.cpp
		src/GpuMesh.cpp
		src/ShaderProgram.cpp
		src/Sampler.cpp
		src/Texture2D.cpp
		src/VertexArray.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/ForgeSim/Renderer/OpenGL/Buffer.hpp
			include/ForgeSim/Renderer/OpenGL/DebugLineRenderer.hpp
			include/ForgeSim/Renderer/OpenGL/GpuMesh.hpp
			include/ForgeSim/Renderer/OpenGL/Sampler.hpp
			include/ForgeSim/Renderer/OpenGL/ShaderProgram.hpp
			include/ForgeSim/Renderer/OpenGL/Texture2D.hpp
			include/ForgeSim/Renderer/OpenGL/VertexArray.hpp
)

target_link_libraries(
	ForgeSimRendererOpenGL
	PUBLIC
		ForgeSim::Assets
		ForgeSim::Renderer
		glm::glm-header-only
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