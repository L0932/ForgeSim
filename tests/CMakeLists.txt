add_executable(ForgeSimCoreSmokeTests)

target_sources(
	ForgeSimCoreSmokeTests
	PRIVATE
		CoreSmokeTests.cpp
)

target_link_libraries(
	ForgeSimCoreSmokeTests
	PRIVATE
		ForgeSim::Core
)

target_compile_features(
	ForgeSimCoreSmokeTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimCoreSmokeTests)

add_test(
	NAME ForgeSim.CoreSmoke
	COMMAND ForgeSimCoreSmokeTests
)


add_executable(ForgeSimFixedStepSchedulerTests)

target_sources(
	ForgeSimFixedStepSchedulerTests
	PRIVATE
		FixedStepSchedulerTest.cpp
)

target_link_libraries(
	ForgeSimFixedStepSchedulerTests
	PRIVATE
		ForgeSim::Core
)

target_compile_features(
	ForgeSimFixedStepSchedulerTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimFixedStepSchedulerTests)

add_test(
	NAME ForgeSim.Core.FixedStepScheduler
	COMMAND ForgeSimFixedStepSchedulerTests
)