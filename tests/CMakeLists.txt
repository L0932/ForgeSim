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
		${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
)

target_include_directories(
	ForgeSimTransformTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimTransformTests
	PRIVATE
		ForgeSim::Renderer
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

add_executable(ForgeSimDebugGeometryTests)

target_sources(
	ForgeSimDebugGeometryTests
	PRIVATE
		DebugGeometryTest.cpp
)

target_link_libraries(
	ForgeSimDebugGeometryTests
	PRIVATE
		ForgeSim::Renderer
)

target_compile_features(
	ForgeSimDebugGeometryTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimDebugGeometryTests)

set_target_properties(
	ForgeSimDebugGeometryTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Renderer.DebugGeometry
	COMMAND ForgeSimDebugGeometryTests
)

add_executable(ForgeSimShaderFileMonitorTests)

target_sources(
	ForgeSimShaderFileMonitorTests
	PRIVATE
		ShaderFileMonitorTest.cpp
		${PROJECT_SOURCE_DIR}/apps/Sandbox/ShaderFileMonitor.cpp
)

target_include_directories(
	ForgeSimShaderFileMonitorTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_compile_features(
	ForgeSimShaderFileMonitorTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimShaderFileMonitorTests
)

set_target_properties(
	ForgeSimShaderFileMonitorTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.ShaderFileMonitor
	COMMAND ForgeSimShaderFileMonitorTests
)

add_executable(ForgeSimRuntimeDiagnosticsTests)

target_sources(
	ForgeSimRuntimeDiagnosticsTests
	PRIVATE
		RuntimeDiagnosticsTest.cpp
)

target_link_libraries(
	ForgeSimRuntimeDiagnosticsTests
	PRIVATE
		ForgeSim::Core
)

target_compile_features(
	ForgeSimRuntimeDiagnosticsTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimRuntimeDiagnosticsTests
)

set_target_properties(
	ForgeSimRuntimeDiagnosticsTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Core.RuntimeDiagnostics
	COMMAND ForgeSimRuntimeDiagnosticsTests
)

add_executable(ForgeSimImageLoaderTests)

target_sources(
	ForgeSimImageLoaderTests
	PRIVATE
		ImageLoaderTest.cpp
)

target_link_libraries(
	ForgeSimImageLoaderTests
	PRIVATE
		ForgeSim::Assets
)

target_compile_features(
	ForgeSimImageLoaderTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimImageLoaderTests
)

set_target_properties(
	ForgeSimImageLoaderTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Assets.ImageLoader
	COMMAND ForgeSimImageLoaderTests
)

add_executable(ForgeSimMaterialDataTests)

target_sources(
	ForgeSimMaterialDataTests
	PRIVATE
		MaterialDataTest.cpp
)

target_link_libraries(
	ForgeSimMaterialDataTests
	PRIVATE
		ForgeSim::Assets
)

target_compile_features(
	ForgeSimMaterialDataTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimMaterialDataTests
)

set_target_properties(
	ForgeSimMaterialDataTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Assets.MaterialData
	COMMAND ForgeSimMaterialDataTests
)

add_executable(ForgeSimMeshProcessingTests)

target_sources(
	ForgeSimMeshProcessingTests
	PRIVATE
		MeshProcessingTest.cpp
)

target_link_libraries(
	ForgeSimMeshProcessingTests
	PRIVATE
		ForgeSim::Assets
)

target_compile_features(
	ForgeSimMeshProcessingTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimMeshProcessingTests
)

set_target_properties(
	ForgeSimMeshProcessingTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Assets.MeshProcessing
	COMMAND ForgeSimMeshProcessingTests
)

add_executable(ForgeSimSceneLightTests)

target_sources(
    ForgeSimSceneLightTests
    PRIVATE
        SceneLightTest.cpp
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
)

target_include_directories(
    ForgeSimSceneLightTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneLightTests
    PRIVATE
        ForgeSim::Renderer
        glm::glm
)

target_compile_features(
    ForgeSimSceneLightTests
    PRIVATE
        cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimSceneLightTests)

set_target_properties(
    ForgeSimSceneLightTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneLight
    COMMAND ForgeSimSceneLightTests
)

add_executable(ForgeSimSceneSelectionTests)

target_sources(
    ForgeSimSceneSelectionTests
    PRIVATE
        SceneSelectionTest.cpp
)

target_include_directories(
    ForgeSimSceneSelectionTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneSelectionTests
    PRIVATE
        ForgeSim::Renderer
        glm::glm
)

target_compile_features(
    ForgeSimSceneSelectionTests
    PRIVATE
        cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimSceneSelectionTests)

set_target_properties(
    ForgeSimSceneSelectionTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneSelection
    COMMAND ForgeSimSceneSelectionTests
)

add_executable(ForgeSimLightPickingTests)

target_sources(
    ForgeSimLightPickingTests
    PRIVATE
        LightPickingTest.cpp
)

target_include_directories(
    ForgeSimLightPickingTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimLightPickingTests
    PRIVATE
        ForgeSim::Renderer
        glm::glm
)

target_compile_features(
    ForgeSimLightPickingTests
    PRIVATE
        cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimLightPickingTests)

set_target_properties(
    ForgeSimLightPickingTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.LightPicking
    COMMAND ForgeSimLightPickingTests
)

add_executable(ForgeSimSceneSelectionValidationTests)

target_sources(
	ForgeSimSceneSelectionValidationTests
	PRIVATE
		SceneSelectionValidationTest.cpp
		${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
)

target_include_directories(
	ForgeSimSceneSelectionValidationTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimSceneSelectionValidationTests
	PRIVATE
		ForgeSim::Renderer
		glm::glm
)

target_compile_features(
	ForgeSimSceneSelectionValidationTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(
	ForgeSimSceneSelectionValidationTests
)

set_target_properties(
	ForgeSimSceneSelectionValidationTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.SceneSelectionValidation
	COMMAND ForgeSimSceneSelectionValidationTests
)

add_executable(
	ForgeSimGizmoTransformTests
	GizmoTransformTest.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/GizmoTransform.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
)

target_include_directories(
	ForgeSimGizmoTransformTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimGizmoTransformTests
	PRIVATE
		ForgeSim::Renderer
		glm::glm
)

target_compile_definitions(
	ForgeSimGizmoTransformTests
	PRIVATE
		GLM_ENABLE_EXPERIMENTAL
)

target_compile_features(
	ForgeSimGizmoTransformTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimGizmoTransformTests)

set_target_properties(
	ForgeSimGizmoTransformTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.GizmoTransform
	COMMAND ForgeSimGizmoTransformTests
)

add_executable(
	ForgeSimSceneEditHistoryTests
	SceneEditHistoryTest.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneEditHistory.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
)

target_include_directories(
	ForgeSimSceneEditHistoryTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimSceneEditHistoryTests
	PRIVATE
		ForgeSim::Renderer
		glm::glm
)

target_compile_features(
	ForgeSimSceneEditHistoryTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimSceneEditHistoryTests)

set_target_properties(
	ForgeSimSceneEditHistoryTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_executable(
	ForgeSimLightGizmoTransformTests
	LightGizmoTransformTest.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/LightGizmoTransform.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
)

target_include_directories(
	ForgeSimLightGizmoTransformTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimLightGizmoTransformTests
	PRIVATE
		ForgeSim::Renderer
		glm::glm
)

target_compile_definitions(
	ForgeSimLightGizmoTransformTests
	PRIVATE
		GLM_ENABLE_EXPERIMENTAL
)

target_compile_features(
	ForgeSimLightGizmoTransformTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimLightGizmoTransformTests)

set_target_properties(
	ForgeSimLightGizmoTransformTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.LightGizmoTransform
	COMMAND ForgeSimLightGizmoTransformTests
)

add_test(
	NAME ForgeSim.Sandbox.SceneEditHistory
	COMMAND ForgeSimSceneEditHistoryTests
)

add_executable(
	ForgeSimLightSubmissionTests
	LightSubmissionTest.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/LightSubmission.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
)

target_include_directories(
	ForgeSimLightSubmissionTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimLightSubmissionTests
	PRIVATE
		glm::glm
)

target_compile_features(
	ForgeSimLightSubmissionTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimLightSubmissionTests)

set_target_properties(
	ForgeSimLightSubmissionTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.LightSubmission
	COMMAND ForgeSimLightSubmissionTests
)

add_executable(
	ForgeSimSceneLightValidationTests
	SceneLightValidationTest.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
	${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightEditing.cpp
)

target_include_directories(
	ForgeSimSceneLightValidationTests
	PRIVATE
		${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
	ForgeSimSceneLightValidationTests
	PRIVATE
		glm::glm
)

target_compile_features(
	ForgeSimSceneLightValidationTests
	PRIVATE
		cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimSceneLightValidationTests)

set_target_properties(
	ForgeSimSceneLightValidationTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)

add_test(
	NAME ForgeSim.Sandbox.SceneLightValidation
	COMMAND ForgeSimSceneLightValidationTests
)

add_executable(
    ForgeSimSceneDocumentValidationTests
    SceneDocumentValidationTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
)

target_include_directories(
    ForgeSimSceneDocumentValidationTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneDocumentValidationTests
    PRIVATE
        glm::glm-header-only
)

target_compile_features(
    ForgeSimSceneDocumentValidationTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(
    ForgeSimSceneDocumentValidationTests
)

set_target_properties(
    ForgeSimSceneDocumentValidationTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneDocumentValidation
    COMMAND ForgeSimSceneDocumentValidationTests
)

add_executable(
    ForgeSimSceneSerializerTests
    SceneSerializerTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
)

target_include_directories(
    ForgeSimSceneSerializerTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneSerializerTests
    PRIVATE
        glm::glm-header-only
        nlohmann_json::nlohmann_json
)

target_compile_features(
    ForgeSimSceneSerializerTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimSceneSerializerTests)

set_target_properties(
    ForgeSimSceneSerializerTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneSerializer
    COMMAND ForgeSimSceneSerializerTests
)

add_executable(
    ForgeSimSceneDocumentIOTests
    SceneDocumentIOTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentIO.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
)

target_include_directories(
    ForgeSimSceneDocumentIOTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneDocumentIOTests
    PRIVATE
        glm::glm-header-only
        nlohmann_json::nlohmann_json
)

target_compile_features(
    ForgeSimSceneDocumentIOTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimSceneDocumentIOTests)

set_target_properties(
    ForgeSimSceneDocumentIOTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneDocumentIO
    COMMAND ForgeSimSceneDocumentIOTests
)

add_executable(
    ForgeSimSandboxSceneDocumentTests
    SandboxSceneDocumentTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
)

target_include_directories(
    ForgeSimSandboxSceneDocumentTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSandboxSceneDocumentTests
    PRIVATE
        glm::glm-header-only
        nlohmann_json::nlohmann_json
)

target_compile_features(
    ForgeSimSandboxSceneDocumentTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(
    ForgeSimSandboxSceneDocumentTests
)

set_target_properties(
    ForgeSimSandboxSceneDocumentTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneDocumentConversion
    COMMAND ForgeSimSandboxSceneDocumentTests
)

add_executable(
    ForgeSimSceneBoundsTests
    SceneBoundsTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneBounds.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
)

target_include_directories(
    ForgeSimSceneBoundsTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneBoundsTests
    PRIVATE
        ForgeSim::Renderer
        glm::glm-header-only
)

target_compile_features(
    ForgeSimSceneBoundsTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimSceneBoundsTests)

set_target_properties(
    ForgeSimSceneBoundsTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneBounds
    COMMAND ForgeSimSceneBoundsTests
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
	ForgeSimSceneLightValidationTests
	ForgeSimTransformTests
	ForgeSimDebugGeometryTests
	ForgeSimShaderFileMonitorTests
	ForgeSimRuntimeDiagnosticsTests
	ForgeSimImageLoaderTests
	ForgeSimMaterialDataTests
	ForgeSimMeshProcessingTests
	ForgeSimSceneLightTests
	ForgeSimSceneSelectionTests
	ForgeSimLightPickingTests
	ForgeSimSceneSelectionValidationTests
	ForgeSimGizmoTransformTests
	ForgeSimSceneEditHistoryTests
	ForgeSimSceneSerializerTests
	ForgeSimLightGizmoTransformTests
	ForgeSimLightSubmissionTests
	ForgeSimSceneDocumentValidationTests
	ForgeSimSceneDocumentIOTests
	ForgeSimSandboxSceneDocumentTests
	ForgeSimSceneBoundsTests
	PROPERTIES
		FOLDER "ForgeSim/Tests"
)