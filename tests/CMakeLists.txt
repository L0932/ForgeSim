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

add_executable(ForgeSimPlatformGLFWTests)

target_sources(
	ForgeSimPlatformGLFWTests
	PRIVATE
		FramebufferExtentTest.cpp
)

target_link_libraries(
	ForgeSimPlatformGLFWTests
	PRIVATE
		ForgeSim::PlatformGLFW
)

target_compile_features(
	ForgeSimPlatformGLFWTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimPlatformGLFWTests)

add_test(
	NAME ForgeSim.PlatformGLFW.FramebufferExtent
	COMMAND ForgeSimPlatformGLFWTests
)

add_executable(ForgeSimPerspectiveCameraTests)

target_sources(
	ForgeSimPerspectiveCameraTests
	PRIVATE
		PerspectiveCameraTest.cpp
)

target_link_libraries(
	ForgeSimPerspectiveCameraTests
	PRIVATE
		ForgeSim::Renderer
)

target_compile_features(
	ForgeSimPerspectiveCameraTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimPerspectiveCameraTests
)

add_test(
	NAME ForgeSim.Renderer.PerspectiveCamera
	COMMAND ForgeSimPerspectiveCameraTests
)

add_executable(ForgeSimInputStateTests)

target_sources(
	ForgeSimInputStateTests
	PRIVATE
		InputStateTest.cpp
)

target_link_libraries(
	ForgeSimInputStateTests
	PRIVATE
		ForgeSim::Platform
)

target_compile_features(
	ForgeSimInputStateTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimInputStateTests
)

add_test(
	NAME ForgeSim.Platform.InputState
	COMMAND ForgeSimInputStateTests
)

add_executable(ForgeSimFreeCameraControllerTests)

target_sources(
	ForgeSimFreeCameraControllerTests
	PRIVATE
		FreeCameraControllerTest.cpp
		${PROJECT_SOURCE_DIR}/apps/Sandbox/FreeCameraController.cpp
)

target_include_directories(
	ForgeSimFreeCameraControllerTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimFreeCameraControllerTests
	PRIVATE
		ForgeSim::Platform
		ForgeSim::Renderer
)

target_compile_features(
	ForgeSimFreeCameraControllerTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimFreeCameraControllerTests
)

add_test(
	NAME ForgeSim.Sandbox.FreeCameraController
	COMMAND ForgeSimFreeCameraControllerTests
)

set_target_properties(
	ForgeSimFreeCameraControllerTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

set_target_properties(
	ForgeSimCoreSmokeTests
	ForgeSimFixedStepSchedulerTests
	ForgeSimPlatformGLFWTests
	ForgeSimPerspectiveCameraTests
	ForgeSimInputStateTests
	ForgeSimFreeCameraControllerTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)
