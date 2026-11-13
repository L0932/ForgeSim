add_executable(ForgeSimFxaaSettingsTests)

target_sources(
    ForgeSimFxaaSettingsTests
    PRIVATE
        FxaaSettingsTest.cpp
)

target_link_libraries(
    ForgeSimFxaaSettingsTests
    PRIVATE
        ForgeSim::Renderer
)

target_compile_features(
    ForgeSimFxaaSettingsTests
    PRIVATE
        cxx_std_20
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimFxaaSettingsTests)

set_target_properties(
    ForgeSimFxaaSettingsTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Renderer.FxaaSettings
    COMMAND ForgeSimFxaaSettingsTests
)

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

set_target_properties(
    ForgeSimCoreSmokeTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

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

set_target_properties(
    ForgeSimFixedStepSchedulerTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

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

set_target_properties(
    ForgeSimPlatformGLFWTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

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

set_target_properties(
    ForgeSimPerspectiveCameraTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
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

set_target_properties(
    ForgeSimInputStateTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
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

set_target_properties(
    ForgeSimFreeCameraControllerTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
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
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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

set_target_properties(
    ForgeSimTransformTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
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
        GltfImageCacheTest.cpp
        GltfTextureFallbackTest.cpp
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

add_executable(ForgeSimTextureTransformTests)

target_sources(
    ForgeSimTextureTransformTests
    PRIVATE
        TextureTransformTest.cpp
)

target_link_libraries(
    ForgeSimTextureTransformTests
    PRIVATE
        ForgeSim::Assets
        nlohmann_json::nlohmann_json
)

target_compile_features(
    ForgeSimTextureTransformTests
    PRIVATE
        cxx_std_20
)

target_compile_definitions(
    ForgeSimTextureTransformTests
    PRIVATE
        FORGESIM_TEST_ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/assets"
        FORGESIM_TEXTURE_TRANSFORM_VISUAL_ASSET="${PROJECT_SOURCE_DIR}/apps/Sandbox/assets/models/TextureTransformTest.gltf"
)

FORGESIM_SET_PROJECT_WARNINGS(ForgeSimTextureTransformTests)

set_target_properties(
    ForgeSimTextureTransformTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Assets.TextureTransform
    COMMAND ForgeSimTextureTransformTests
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
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
        ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/EditorSession.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
)

target_include_directories(
    ForgeSimSceneDocumentIOTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
        ${PROJECT_SOURCE_DIR}/engine/Renderer/include
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
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
)

target_include_directories(
    ForgeSimSandboxSceneDocumentTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
        ${PROJECT_SOURCE_DIR}/engine/Renderer/include
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

add_executable(
    ForgeSimDirectionalShadowProjectionTests
    DirectionalShadowProjectionTest.cpp
)

target_link_libraries(
    ForgeSimDirectionalShadowProjectionTests
    PRIVATE
        ForgeSim::Renderer
)

target_compile_features(
    ForgeSimDirectionalShadowProjectionTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimDirectionalShadowProjectionTests)

set_target_properties(
    ForgeSimDirectionalShadowProjectionTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Renderer.DirectionalShadowProjection
    COMMAND ForgeSimDirectionalShadowProjectionTests
)

add_executable(
    ForgeSimSceneCameraValidationTests
    SceneCameraValidationTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
)

target_include_directories(
    ForgeSimSceneCameraValidationTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneCameraValidationTests
    PRIVATE
        glm::glm-header-only
)

target_compile_features(
    ForgeSimSceneCameraValidationTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimSceneCameraValidationTests)

set_target_properties(
    ForgeSimSceneCameraValidationTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneCameraValidation
    COMMAND ForgeSimSceneCameraValidationTests
)

add_executable(
    ForgeSimSceneRenderingValidationTests
    SceneRenderingValidationTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
)

target_include_directories(
    ForgeSimSceneRenderingValidationTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimSceneRenderingValidationTests
    PRIVATE
        glm::glm-header-only
)

target_compile_features(
    ForgeSimSceneRenderingValidationTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(
    ForgeSimSceneRenderingValidationTests
)

set_target_properties(
    ForgeSimSceneRenderingValidationTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.SceneRenderingValidation
    COMMAND ForgeSimSceneRenderingValidationTests
)

add_executable(
    ForgeSimShowcasePersistenceTests
    ShowcasePersistenceTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/EditorSession.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraState.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneEnvironment.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentIO.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
)

target_include_directories(
    ForgeSimShowcasePersistenceTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/Sandbox
)

target_link_libraries(
    ForgeSimShowcasePersistenceTests
    PRIVATE
        ForgeSim::Renderer
        nlohmann_json::nlohmann_json
)

target_compile_features(
    ForgeSimShowcasePersistenceTests
    PRIVATE
        cxx_std_20
)

forgesim_set_project_warnings(ForgeSimShowcasePersistenceTests)

set_target_properties(
    ForgeSimShowcasePersistenceTests
    PROPERTIES
        FOLDER "ForgeSim/Tests"
)

add_test(
    NAME ForgeSim.Sandbox.ShowcasePersistence
    COMMAND ForgeSimShowcasePersistenceTests
)

# Imported-node transforms are pure math and need no graphics context.
add_executable(ForgeSimSceneNodeTransformTests SceneNodeTransformTest.cpp)
target_include_directories(ForgeSimSceneNodeTransformTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimSceneNodeTransformTests PRIVATE
    ForgeSim::Assets glm::glm-header-only)
target_compile_definitions(ForgeSimSceneNodeTransformTests PRIVATE
    FORGESIM_TEST_ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_compile_features(ForgeSimSceneNodeTransformTests PRIVATE cxx_std_20)
FORGESIM_SET_PROJECT_WARNINGS(ForgeSimSceneNodeTransformTests)
set_target_properties(ForgeSimSceneNodeTransformTests PROPERTIES
    FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.SceneNodeTransform
    COMMAND ForgeSimSceneNodeTransformTests)


add_executable(ForgeSimConsoleCommandsTests ConsoleCommandsTest.cpp)
target_include_directories(ForgeSimConsoleCommandsTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_compile_features(ForgeSimConsoleCommandsTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimConsoleCommandsTests)
set_target_properties(ForgeSimConsoleCommandsTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ConsoleCommands COMMAND ForgeSimConsoleCommandsTests)

add_executable(ForgeSimConsoleProjectRootTests
    ConsoleProjectRootTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/EditorSession.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentIO.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp)
target_include_directories(ForgeSimConsoleProjectRootTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox"
    "${PROJECT_SOURCE_DIR}/engine/Renderer/include"
)
target_link_libraries(ForgeSimConsoleProjectRootTests PRIVATE
    glm::glm-header-only nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimConsoleProjectRootTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimConsoleProjectRootTests)
set_target_properties(ForgeSimConsoleProjectRootTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ConsoleProjectRoot COMMAND ForgeSimConsoleProjectRootTests)

add_executable(ForgeSimBundledSamplesTests
    BundledSamplesTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/EditorSession.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentIO.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp)
target_include_directories(ForgeSimBundledSamplesTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox"
    "${PROJECT_SOURCE_DIR}/engine/Renderer/include"
)
target_link_libraries(ForgeSimBundledSamplesTests PRIVATE
    glm::glm-header-only nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimBundledSamplesTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimBundledSamplesTests)
set_target_properties(ForgeSimBundledSamplesTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.BundledSamples COMMAND ForgeSimBundledSamplesTests)

target_compile_definitions(ForgeSimBundledSamplesTests PRIVATE
    FORGESIM_SAMPLE_ASSET_DIRECTORY="${PROJECT_SOURCE_DIR}/apps/Sandbox/assets")

add_executable(ForgeSimExternalSampleTests ExternalSampleTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp)
target_link_libraries(ForgeSimExternalSampleTests PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(ForgeSimExternalSampleTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_compile_features(ForgeSimExternalSampleTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimExternalSampleTests)
set_target_properties(ForgeSimExternalSampleTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ExternalSample COMMAND ForgeSimExternalSampleTests)

add_executable(ForgeSimSceneAssetRootTests
    SceneAssetRootTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp)
target_include_directories(ForgeSimSceneAssetRootTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimSceneAssetRootTests PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimSceneAssetRootTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimSceneAssetRootTests)
set_target_properties(ForgeSimSceneAssetRootTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.SceneAssetRoot COMMAND ForgeSimSceneAssetRootTests)

add_executable(ForgeSimMaterialUsageTests MaterialUsageTest.cpp)
target_include_directories(ForgeSimMaterialUsageTests PRIVATE
    "${PROJECT_SOURCE_DIR}/apps/Sandbox"
    "${PROJECT_SOURCE_DIR}/engine/Assets/include")
target_link_libraries(ForgeSimMaterialUsageTests PRIVATE glm::glm-header-only)
target_compile_features(ForgeSimMaterialUsageTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimMaterialUsageTests)
set_target_properties(ForgeSimMaterialUsageTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.MaterialUsage COMMAND ForgeSimMaterialUsageTests)

add_executable(ForgeSimModelImportTests ModelImportTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp)
target_include_directories(ForgeSimModelImportTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimModelImportTests PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimModelImportTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimModelImportTests)
set_target_properties(ForgeSimModelImportTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ModelImport COMMAND ForgeSimModelImportTests)

add_executable(ForgeSimCameraNavigationTests CameraNavigationTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraFraming.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/FreeCameraController.cpp)
target_include_directories(ForgeSimCameraNavigationTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimCameraNavigationTests PRIVATE ForgeSim::Platform ForgeSim::Renderer)
target_compile_features(ForgeSimCameraNavigationTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimCameraNavigationTests)
set_target_properties(ForgeSimCameraNavigationTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.CameraNavigation COMMAND ForgeSimCameraNavigationTests)

add_executable(ForgeSimSelectionFocusTests SelectionFocusTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneBounds.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraFraming.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/FreeCameraController.cpp)
target_include_directories(ForgeSimSelectionFocusTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimSelectionFocusTests PRIVATE ForgeSim::Platform ForgeSim::Renderer)
target_compile_features(ForgeSimSelectionFocusTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimSelectionFocusTests)
set_target_properties(ForgeSimSelectionFocusTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.SelectionFocus COMMAND ForgeSimSelectionFocusTests)

add_executable(ForgeSimSceneDeletionTests SceneDeletionTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneEditHistory.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/EditorSession.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentIO.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneAssetRoot.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp)
target_include_directories(ForgeSimSceneDeletionTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimSceneDeletionTests PRIVATE ForgeSim::Renderer nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimSceneDeletionTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimSceneDeletionTests)
set_target_properties(ForgeSimSceneDeletionTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.SceneDeletion COMMAND ForgeSimSceneDeletionTests)

add_executable(ForgeSimObjectMultiSelectionTests ObjectMultiSelectionTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneTransformEditor.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/LightGizmoTransform.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneEditHistory.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxSceneDocument.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneSerializer.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneDocumentValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneBounds.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/GizmoTransform.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/ViewportSelectionController.cpp)
target_include_directories(ForgeSimObjectMultiSelectionTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimObjectMultiSelectionTests PRIVATE ForgeSim::PlatformGLFW ForgeSim::Renderer nlohmann_json::nlohmann_json)
target_compile_features(ForgeSimObjectMultiSelectionTests PRIVATE cxx_std_20)
target_compile_definitions(ForgeSimObjectMultiSelectionTests PRIVATE GLM_ENABLE_EXPERIMENTAL)
forgesim_set_project_warnings(ForgeSimObjectMultiSelectionTests)
set_target_properties(ForgeSimObjectMultiSelectionTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ObjectMultiSelection COMMAND ForgeSimObjectMultiSelectionTests)

add_executable(ForgeSimModelNodeSelectionTests ModelNodeSelectionTest.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/ViewportSelectionController.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneEditHistory.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SandboxScene.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneCameraValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneLightValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/SceneRenderingValidation.cpp
    ${PROJECT_SOURCE_DIR}/apps/Sandbox/Transform.cpp)
target_include_directories(ForgeSimModelNodeSelectionTests PRIVATE "${PROJECT_SOURCE_DIR}/apps/Sandbox")
target_link_libraries(ForgeSimModelNodeSelectionTests PRIVATE ForgeSim::Assets ForgeSim::Renderer ForgeSim::PlatformGLFW)
target_compile_features(ForgeSimModelNodeSelectionTests PRIVATE cxx_std_20)
forgesim_set_project_warnings(ForgeSimModelNodeSelectionTests)
set_target_properties(ForgeSimModelNodeSelectionTests PROPERTIES FOLDER "ForgeSim/Tests")
add_test(NAME ForgeSim.Sandbox.ModelNodeSelection COMMAND ForgeSimModelNodeSelectionTests)
