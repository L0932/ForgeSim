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

add_executable(ForgeSimTransformTests)

target_sources(
	ForgeSimTransformTests
	PRIVATE
		TransformTest.cpp
		${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
)

target_include_directories(
	ForgeSimTransformTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimTransformTests
	PRIVATE
		glm::glm
)

target_compile_features(
	ForgeSimTransformTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimTransformTests
)

add_test(
	NAME ForgeSim.Sandbox.Transform
	COMMAND ForgeSimTransformTests
)

add_executable(ForgeSimGltfLoaderTests)

target_sources(
	ForgeSimGltfLoaderTests
	PRIVATE
		GltfLoaderTest.cpp
)

target_link_libraries(
	ForgeSimGltfLoaderTests
	PRIVATE
		ForgeSim::Assets
)

target_compile_features(
	ForgeSimGltfLoaderTests
	PRIVATE
		cxx_std_20
)

target_compile_definitions(
	ForgeSimGltfLoaderTests
	PRIVATE
		FORGESIM_TEST_ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimGltfLoaderTests)

set_target_properties(
	ForgeSimGltfLoaderTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Assets.GltfLoader
	COMMAND ForgeSimGltfLoaderTests
)

add_executable(ForgeSimRendererPickingTests)

target_sources(
	ForgeSimRendererPickingTests
	PRIVATE
		PickingTest.cpp
)

target_link_libraries(
	ForgeSimRendererPickingTests
	PRIVATE
		ForgeSim::Renderer
)

target_compile_features(
	ForgeSimRendererPickingTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimRendererPickingTests
)

set_target_properties(
	ForgeSimRendererPickingTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Renderer.Picking
	COMMAND ForgeSimRendererPickingTests
)

set_target_properties(
	ForgeSimCoreSmokeTests
	ForgeSimFixedStepSchedulerTests
	ForgeSimFreeCameraControllerTests
	ForgeSimGltfLoaderTests
	ForgeSimInputStateTests
	ForgeSimPerspectiveCameraTests
	ForgeSimPlatformGLFWTests
	ForgeSimRendererPickingTests
	ForgeSimTransformTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)
