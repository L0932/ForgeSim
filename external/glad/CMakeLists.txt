add_library(
	ForgeSimGlad
	STATIC
	src/gl.c
)

add_library(
	ForgeSim::Glad
	ALIAS 
	ForgeSimGlad
)

target_include_directories(
	ForgeSimGlad
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(
	ForgeSimGlad
	PROPERTIES
		C_STANDARD 11
		C_STANDARD_REQUIRED YES
		C_EXTENSIONS NO 
)