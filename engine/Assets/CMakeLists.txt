add_library(ForgeSimAssets STATIC)

add_library(
	ForgeSim::Assets
	ALIAS
		ForgeSimAssets
)

target_sources(
	ForgeSimAssets
	PRIVATE
		src/GltfLoader.cpp
		src/ImageLoader.cpp
		src/MeshProcessing.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/ForgeSim/Assets/GltfLoader.hpp
			include/ForgeSim/Assets/ImageLoader.hpp
			include/ForgeSim/Assets/MaterialData.hpp
			include/ForgeSim/Assets/MeshData.hpp
			include/ForgeSim/Assets/MeshProcessing.hpp
			include/ForgeSim/Assets/TextureData.hpp
)

target_link_libraries(
	ForgeSimAssets
	PRIVATE
		ForgeSimStb
		ForgeSim::Cgltf
		glm::glm-header-only
)

target_compile_features(
	ForgeSimAssets
	PUBLIC
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimAssets)

set_target_properties(
	ForgeSimAssets
	PROPERTIES
		FOLDER "ForgeSim/Assets"
)