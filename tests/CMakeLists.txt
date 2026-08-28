add_executable(ForgeSimCoreTests)

target_sources(
	ForgeSimCoreTests
	PRIVATE
		CoreSmokeTests.cpp
)

target_link_libraries(
	ForgeSimCoreTests
	PRIVATE
		ForgeSim::Core
)

target_compile_features(
	ForgeSimCoreTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimCoreTests)

add_test(
	NAME ForgeSim.CoreSmoke
	COMMAND ForgeSimCoreTests
)