#include "SceneAssetPath.hpp"
#include "FreeCameraController.hpp"
#include "BundledSamples.hpp"
#include "ExternalSample.hpp"
#include "SceneAssetRoot.hpp"
#include "SceneAssetRecovery.hpp"
#include "ConsoleProjectRoot.hpp"
#include "ConsoleTextureFallback.hpp"
#include "ConsoleImageCache.hpp"
#include "ConsoleLoadingDiagnostics.hpp"
#include "LightDebugLayout.hpp"
#include "LightPicking.hpp"
#include "SandboxApplication.hpp"
#include "SandboxPaths.hpp"
#include "SceneCameraFraming.hpp"
#include "SceneCameraState.hpp"
#include "SceneEnvironment.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneFilePanel.hpp"
#include "SceneHierarchyPanel.hpp"
#include "SceneInspectorPanel.hpp"
#include "SceneTransformEditor.hpp"
#include "SceneViewportPanel.hpp"
#include "SceneSelection.hpp"
#include "SelectionFocus.hpp"
#include "SceneSelectionValidation.hpp"
#include "TransformEditorUIState.hpp"
#include "ViewportSelectionController.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/RuntimeDiagnostics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>
#include <ForgeSim/Platform/FileDialogs.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        void PresentLoadingFrame(Platform::GlfwWindow& window, SandboxDebugUI& ui,
            const Assets::GltfLoadProgress& progress)
        {
            window.PollEvents();
            const auto extent = window.GetFramebufferExtent();
            if (extent.width > 0 && extent.height > 0)
            {
                ui.DrawLoadingFrame(progress, extent.width, extent.height);
                window.SwapBuffers();
            }
        }

        SandboxRendererSpecification InitialRendererSpecification(
            const SandboxApplicationSpecification& specification,
            Platform::GlfwWindow& window, SandboxDebugUI& ui)
        {
            auto renderer = specification.rendererSpec;
            if (specification.initialScene)
            {
                renderer.environmentFilename = ResolveSceneEnvironment(
                    specification.initialScene->rendering,
                    specification.projectRoot.value_or(BundledAssetDirectory().parent_path()),
                    BundledAssetDirectory() / "environments");
            }
            renderer.drawStartupProgress = [&window, &ui](const Assets::GltfLoadProgress& progress)
            {
                PresentLoadingFrame(window, ui, progress);
            };
            window.Show();
            renderer.drawStartupProgress({"Starting ForgeSim", Assets::GltfProgressPhase::Started,
                0, std::nullopt, std::chrono::milliseconds{0}});
            return renderer;
        }

        void PresentStartupError(Platform::GlfwWindow& window, SandboxDebugUI& ui,
            std::string_view message)
        {
            window.Show();
            const Assets::GltfLoadProgress failed{"Startup failed", Assets::GltfProgressPhase::Started,
                0, std::nullopt, std::chrono::milliseconds{0}};
            while (!window.ShouldClose())
            {
                window.PollEvents();
                if (window.ShouldClose())
                    break;
                const auto extent = window.GetFramebufferExtent();
                if (extent.width <= 0 || extent.height <= 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{16});
                    continue;
                }
                const bool dismissed = ui.DrawLoadingFrame(failed, extent.width, extent.height,
                    message);
                window.SwapBuffers();
                if (dismissed)
                    break;
            }
        }

        SandboxRenderer CreateInitialRenderer(
            const SandboxApplicationSpecification& specification,
            Platform::GlfwWindow& window, SandboxDebugUI& ui)
        {
            try
            {
                return SandboxRenderer{InitialRendererSpecification(specification, window, ui)};
            }
            catch (const std::exception& exception)
            {
                PresentStartupError(window, ui, exception.what());
                throw;
            }
        }
    }

    void SandboxApplication::ShowStartupError(
        const Platform::WindowSpecification& specification, std::string_view message)
    {
        Platform::GlfwWindow window{specification};
        SandboxDebugUI ui{window};
        PresentStartupError(window, ui, message);
    }

	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_DebugUI(m_Window)
		, m_Renderer(CreateInitialRenderer(spec, m_Window, m_DebugUI))
		, m_LoadedScene(spec.initialScene.has_value())
        , m_ExplicitSceneRoot(spec.explicitSceneRoot)
	{
		const std::filesystem::path projectRoot =
			std::filesystem::weakly_canonical(
				std::filesystem::absolute(
					spec.projectRoot.value_or(
						BundledAssetDirectory().parent_path())));

		SandboxScene initialScene =
			spec.initialScene
			? SandboxScene{ *spec.initialScene }
		: SandboxScene{};

		if (!m_LoadedScene)
		{
			SandboxObject& object =
				initialScene.Objects().front();

			const std::u8string name =
				m_Renderer.ObjectModelPath()
				.stem().generic_u8string();

			object.name.assign(name.begin(), name.end());

			const std::u8string reference =
				m_Renderer.ObjectModelPath()
				.lexically_relative(projectRoot)
				.generic_u8string();

			object.modelPath.assign(
				reference.begin(),
				reference.end());

			initialScene.MainDirectionalLight().position =
				CalculateLightDebugLayout(
					m_Renderer.ObjectLocalBounds()).directionalAnchor;
		}

		SceneDocument document = initialScene.ToDocument();

		if (!m_LoadedScene)
		{
			const std::u8string modelName =
				m_Renderer.ObjectModelPath().filename().generic_u8string();

			document.name.assign(
				modelName.begin(),
				modelName.end());
		}

		m_EditorSession = std::make_unique<EditorSession>(
			document,
			projectRoot,
			spec.scenePath,
			DefaultSceneDirectory());

        m_Renderer.LoadSceneModels(projectRoot, m_EditorSession->Scene().Objects(),
            ResolveSceneEnvironment(m_EditorSession->Scene().RenderingSettings(), projectRoot,
                BundledAssetDirectory() / "environments"));

		Core::LogInfo(
			"Scene project root: " + projectRoot.string());
        Core::LogInfo("Scene loaded: " + m_EditorSession->DisplayName());
	}

	void SandboxApplication::Run(std::optional<std::chrono::steady_clock::time_point> startupStart)
	{
		if (m_RunAttempted)
		{
			throw std::logic_error(
				"Run() can only be called once per application instance.");
		}

		m_RunAttempted = true;
		bool firstFramePresented = false;
		
		SandboxScene& scene = m_EditorSession->Scene();

		m_Window.Show();

		const Renderer::AxisAlignedBoundingBox bounds =
			CalculateSceneBounds(
				scene.Objects(),
				m_Renderer.ObjectLocalBounds());

		const glm::vec3 modelCenter =
			(bounds.minimum + bounds.maximum) * 0.5f;

		const float modelRadius =
			std::max(
				glm::length(bounds.maximum - bounds.minimum) * 0.5f,
				0.001f);

		const auto initialExtent =
			m_Window.GetFramebufferExtent();

		const float initialAspectRatio =
			initialExtent.IsDrawable()
			? static_cast<float>(initialExtent.width) /
			static_cast<float>(initialExtent.height)
			: 1.0f;

		const Renderer::AxisAlignedBoundingBox emptySceneBounds{
			.minimum = glm::vec3{ -1.0f },
			.maximum = glm::vec3{ 1.0f }
		};

        SceneCameraFraming initialFraming = scene.Objects().empty()
            ? CreateEmptySceneCameraFraming(initialAspectRatio)
            : CreateSceneCameraFraming(bounds, initialAspectRatio);

		Renderer::PerspectiveCamera camera =
			std::move(initialFraming.camera);

		FreeCameraController cameraController =
			std::move(initialFraming.controller);

		SceneSelection selection;
		ViewportSelectionController selectionController;
		SceneTransformEditor transformEditor;

		ForgeSim::Core::Timer timer;
		ForgeSim::Core::RuntimeDiagnostics runtimeDiagnostics;

		std::optional<
			ForgeSim::Core::RuntimeDiagnosticsSnapshot>
			latestDiagnosticsSnapshot;

		using FixedStepScheduler =
			ForgeSim::Core::FixedStepScheduler;

		const auto fixedStep =
			std::chrono::duration_cast<
			FixedStepScheduler::Duration>(
				std::chrono::seconds{ 1 }) / 60;

		constexpr std::uint32_t maximumStepsPerFrame = 4;

		FixedStepScheduler scheduler{
			fixedStep,
			maximumStepsPerFrame
		};

		if (!m_LoadedScene && scene.PointLights().empty())
		{
			static_cast<void>(scene.AddPointLight());

			PointLight& pointLight =
				scene.PointLights().back();

			const float visualizationScale =
				std::max(modelRadius, 0.001f);

			pointLight.position =
				modelCenter +
				glm::vec3{
					visualizationScale * 0.75f,
					visualizationScale * 0.5f,
					0.0f
			};

			pointLight.range =
				visualizationScale * 0.5f;

			pointLight.intensity = 10.0f;
		}

		if (!m_LoadedScene && scene.SpotLights().empty())
		{
			static_cast<void>(scene.AddSpotLight());

			SpotLight& spotLight =
				scene.SpotLights().back();

			const float visualizationScale =
				std::max(modelRadius, 0.001f);

			spotLight.position =
				modelCenter +
				glm::vec3{
					-visualizationScale * 0.5f,
					visualizationScale * 0.5f,
					0.0f
			};

			spotLight.emissionDirection = {
				0.0f,
				-1.0f,
				0.0f
			};

			spotLight.range =
				visualizationScale * 0.35f;

			spotLight.innerConeAngleRadians =
				glm::radians(15.0f);

			spotLight.outerConeAngleRadians =
				glm::radians(30.0f);

			spotLight.intensity = 10.0f;
		}

		if (!m_LoadedScene)
		{
			m_EditorSession->ResetChangeBaseline();
		}

		std::vector<LightPickTarget> lightPickTargets;

        ConsoleProjectRoot sceneOpenRoot{BundledAssetDirectory().parent_path(), m_ExplicitSceneRoot};
        SceneFilePanel sceneFilePanel;
        sceneFilePanel.RegisterConsoleCommand("project.root",
            "Show or set the project root for subsequent scene opens.",
            "project.root [directory]", 0, 1,
            [&sceneOpenRoot](auto arguments, const auto&)
            { return sceneOpenRoot.Execute(arguments); });
        sceneFilePanel.RegisterConsoleCommand("project.root.auto",
            "Clear the explicit root override and discover roots for subsequent opens.",
            "project.root.auto", 0, 0,
            [&sceneOpenRoot](auto, const auto&)
            {
                sceneOpenRoot.UseAutomaticDiscovery();
                return ConsoleCommandResult{"Automatic project-root discovery enabled for subsequent opens."};
            });
        sceneFilePanel.RegisterConsoleCommand("textures.fallback",
            "Show or set texture fallbacks for subsequent model loads.",
            "textures.fallback [on|off]", 0, 1,
            [this](auto arguments, const auto&)
            {
                bool enabled = m_Renderer.TexturePolicy() == Assets::GltfTexturePolicy::SkipUnusableTextures;
                auto result = ExecuteTextureFallbackCommand(arguments, enabled);
                if (!result.error && !arguments.empty())
                    m_Renderer.SetTexturePolicy(enabled ? Assets::GltfTexturePolicy::SkipUnusableTextures
                        : Assets::GltfTexturePolicy::Strict);
                return result;
            });
        sceneFilePanel.RegisterConsoleCommand("images.cache",
            "Show or set decoded-image caching for subsequent model loads.",
            "images.cache [on|off]", 0, 1,
            [this](auto arguments, const auto&)
            {
                bool enabled = m_Renderer.DecodedImageCaching();
                auto result = ExecuteImageCacheCommand(arguments, enabled);
                if (!result.error && !arguments.empty())
                    m_Renderer.SetDecodedImageCaching(enabled);
                return result;
            });

        sceneFilePanel.RegisterConsoleCommand("loading.diagnostics",
            "Show or set scene-load timing and import-stage diagnostics.",
            "loading.diagnostics [on|off]", 0, 1,
            [this](auto arguments, const auto&)
            {
                bool enabled = m_Renderer.LoadingDiagnostics();
                auto result = ExecuteLoadingDiagnosticsCommand(arguments, enabled);
                if (!result.error && !arguments.empty())
                    m_Renderer.SetLoadingDiagnostics(enabled);
                return result;
            });

		SceneHierarchyPanel hierarchyPanel;
		SceneInspectorPanel inspectorPanel;
		SceneViewportPanel sceneViewportPanel;
		std::string sceneFileMessage;

		enum class PendingSceneAction
		{
			None,
			New,
			Open,
			Close
		};

		PendingSceneAction pendingAction =
			PendingSceneAction::None;

		std::filesystem::path pendingOpenPath;
		bool pendingSample = false;
        std::optional<Assets::GltfTexturePolicy> pendingTexturePolicy;
        bool recoveringAssets = false;
        std::string assetRecoveryMessage;
        std::string missingModelReference;
		std::optional<std::filesystem::path> pendingProjectRoot;

		// Committing a prepared session must not fail after the model changes.
		static_assert(
			std::is_nothrow_move_assignable_v<EditorSession>);

		bool exitRequested = false;
		bool cameraFramingRequested = true;
        bool sceneNavigationResetRequested = true;
        std::optional<SceneCameraData> cameraRestoreRequest;
        std::vector<SandboxObject> visibleObjects;
        std::vector<PointLight> visiblePointLights;
        std::vector<SpotLight> visibleSpotLights;
		bool viewportNavigationActive = false;

		// Empty workspaces use a fixed guide scale instead of tiny placeholder bounds.
        const Renderer::AxisAlignedBoundingBox emptyLightDebugBounds{
            glm::vec3{-2.5f}, glm::vec3{2.5f}};

		// Light guides and their picking radius follow the transformed
		// scene size, rather than the shared mesh's local dimensions.
		Renderer::AxisAlignedBoundingBox lightDebugBounds =
			scene.Objects().empty()
			? emptyLightDebugBounds
			: CalculateSceneBounds(
				scene.Objects(),
				m_Renderer.ObjectLocalBounds());

		while (!exitRequested)
		{
			m_Window.PollEvents();

			if (m_Window.ShouldClose())
			{
				m_Window.SetShouldClose(false);

				// Keep an existing confirmation bound to its operation.
				if (pendingAction == PendingSceneAction::None)
				{
					if (!m_EditorSession->HasUnsavedChanges())
					{
						break;
					}

					sceneFileMessage.clear();
					pendingAction = PendingSceneAction::Close;
				}

				m_Window.SetCursorMode(
					Platform::CursorMode::Normal);
			}

			const bool changePending =
				pendingAction != PendingSceneAction::None;

			m_Renderer.ReloadChangedShaders();

			const Renderer::AxisAlignedBoundingBox editorBounds =
				scene.Objects().empty()
				? emptySceneBounds
				: m_Renderer.ObjectLocalBounds();

			lightPickTargets.clear();

			if (m_DebugUI.ShowLights())
			{
				const LightDebugLayout layout =
					CalculateLightDebugLayout(lightDebugBounds);

                if (scene.HasDirectionalLight())
                {
				lightPickTargets.push_back(
					LightPickTarget{
						.id = scene.MainDirectionalLight().id,
						.bounds = {
							.center = scene.MainDirectionalLight().position,
							.radius = layout.pickRadius
						}
					});
                }

				for (const PointLight& light : scene.PointLights())
				{
					lightPickTargets.push_back(
						LightPickTarget{
							.id = light.id,
							.bounds = {
								.center = light.position,
								.radius = layout.pickRadius
							}
						});
				}

				for (const SpotLight& light : scene.SpotLights())
				{
					lightPickTargets.push_back(
						LightPickTarget{
							.id = light.id,
							.bounds = {
								.center = light.position,
								.radius = layout.pickRadius
							}
						});
				}
			}

			const ForgeSim::Platform::InputState& input =
				m_Window.GetInputState();

			const auto framebufferExtent =
				m_Window.GetFramebufferExtent();

			if (!framebufferExtent.IsDrawable())
			{
				m_Window.WaitEvents();
				static_cast<void>(timer.Restart());
				continue;
			}

			m_DebugUI.BeginFrame();
			transformEditor.BeginFrame();

			sceneFilePanel.DrawMenu(
				!changePending &&
				!transformEditor.IsActive() &&
				!input.GetMouseButtonState(
					Platform::MouseButton::Right).held,
				m_EditorSession->ScenePath().has_value(), scene.HasDirectionalLight());

            SceneToolbarRequest toolbarRequest;
            const bool layoutReset = m_DebugUI.BeginWorkspace([&]
            {
                toolbarRequest = sceneFilePanel.DrawToolbar(SceneToolbarContext{
                    .actionsAllowed = !changePending && !transformEditor.IsActive() &&
                        !input.GetMouseButtonState(Platform::MouseButton::Right).held,
                    .hasScenePath = m_EditorSession->ScenePath().has_value(),
                    .canUndo = transformEditor.CanUndo(),
                    .canRedo = transformEditor.CanRedo(),
                    .sceneName = m_EditorSession->DisplayName(),
                    .dirty = m_EditorSession->HasUnsavedChanges()
                });
            });
            if (layoutReset) sceneFilePanel.ResetSampleBrowserLayout();

			runtimeDiagnostics.SetEnabled(
				m_DebugUI.RuntimeDiagnosticsEnabled());

			if (!runtimeDiagnostics.IsEnabled())
			{
				latestDiagnosticsSnapshot.reset();
			}

			std::string scenePathText;
			std::string sceneDisplayName = m_EditorSession->DisplayName();

			if (m_EditorSession->ScenePath())
			{
				const auto& path = *m_EditorSession->ScenePath();

				const std::u8string utf8Path =
					path.generic_u8string();

				scenePathText.assign(
					utf8Path.begin(),
					utf8Path.end());

				const std::u8string utf8Name =
					path.filename().generic_u8string();

				sceneDisplayName.assign(
					utf8Name.begin(),
					utf8Name.end());
			}

			const SceneFileRequest fileRequest =
				sceneFilePanel.Draw(
					SceneFilePanelContext{
						.sceneName = sceneDisplayName,
						.scenePath = scenePathText,
						.message = sceneFileMessage,
						.dirty =
							m_EditorSession->HasUnsavedChanges(),
						.editingActive = transformEditor.IsActive(),
						.navigationActive =
							input.GetMouseButtonState(
								Platform::MouseButton::Right).held,
						.confirmationPending = changePending && !recoveringAssets,
						.pendingActionText =
							pendingAction == PendingSceneAction::New
								? "creating a new scene"
								: pendingAction == PendingSceneAction::Open
									? "opening another scene"
									: "closing",
                        .recoveryMessage = assetRecoveryMessage,
                        .canLocateModel = !missingModelReference.empty()
					});

			TransformEditorUIState editorUI =
				m_DebugUI.UpdateTransformEditorShortcuts(
					transformEditor.IsActive(),
					sceneFilePanel.ConsoleWantsKeyboard() || changePending ||
					input.GetMouseButtonState(
						Platform::MouseButton::Right).held);


			hierarchyPanel.Draw(
				SceneHierarchyPanelContext{
					.scene = scene,
					.selection = selection,
					.interactionDisabled =
						changePending ||
						transformEditor.IsActive() ||
						input.GetMouseButtonState(
							Platform::MouseButton::Right).held
				});

			const auto windowExtent = m_Window.GetWindowExtent();

			const glm::vec2 framebufferScale{
				windowExtent.IsValid()
					? static_cast<float>(framebufferExtent.width) /
						static_cast<float>(windowExtent.width)
					: 1.0f,

				windowExtent.IsValid()
					? static_cast<float>(framebufferExtent.height) /
						static_cast<float>(windowExtent.height)
					: 1.0f
			};

            SceneViewportState viewport = sceneViewportPanel.Begin(framebufferScale);

			if (viewport.IsDrawable())
			{
				m_Renderer.PrepareSceneTexture(
					static_cast<std::uint32_t>(
						viewport.framebufferExtent.width),
					static_cast<std::uint32_t>(
						viewport.framebufferExtent.height));

				sceneViewportPanel.DrawImage(
					m_Renderer.SceneColorTexture());
			}

            editorUI = m_DebugUI.DrawTransformEditorToolbar(viewport,
                changePending || transformEditor.IsActive() ||
                input.GetMouseButtonState(Platform::MouseButton::Right).held);

			const bool leftMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Left).held;

			const bool rightMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Right).held;

			const bool previouslyNavigating = viewportNavigationActive;

			if (!rightMouseDown ||
                sceneFilePanel.ConsoleWantsKeyboard() ||
				changePending ||
				!viewport.IsDrawable())
			{
				viewportNavigationActive = false;
			}
			else if (
				input.GetMouseButtonState(
					Platform::MouseButton::Right).pressed &&
				viewport.hovered &&
				!transformEditor.IsActive() &&
				!leftMouseDown)
			{
				viewportNavigationActive = true;
			}

			if (viewportNavigationActive != previouslyNavigating)
			{
				m_Window.SetCursorMode(
					viewportNavigationActive
					? Platform::CursorMode::Captured
					: Platform::CursorMode::Normal);
			}

			const float aspectRatio =
				viewport.IsDrawable()
				? static_cast<float>(
					viewport.framebufferExtent.width) /
				static_cast<float>(
					viewport.framebufferExtent.height)
				: 1.0f;

            // A pose restore can bypass the framing branch. Reset navigation first
            // so it always belongs to the new scene, including empty scenes/imports.
            if (sceneNavigationResetRequested)
            {
                const auto navigationBounds = scene.Objects().empty() ? emptySceneBounds :
                    CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());
                cameraController = CreateSceneCameraController(navigationBounds, scene.CameraMovementSpeed());
                sceneNavigationResetRequested = false;
                viewportNavigationActive = false;
                m_Window.SetCursorMode(Platform::CursorMode::Normal);
            }

            if (cameraRestoreRequest && viewport.IsDrawable())
            {
                camera = CreateSceneCamera(*cameraRestoreRequest);
                cameraRestoreRequest.reset();
                cameraFramingRequested = false;
                viewportNavigationActive = false;
                m_Window.SetCursorMode(Platform::CursorMode::Normal);
            }

			if (cameraFramingRequested && viewport.IsDrawable())
			{
				cameraFramingRequested = false;

				try
				{
					const Renderer::AxisAlignedBoundingBox framingBounds =
						scene.Objects().empty()
						? emptySceneBounds
						: CalculateSceneBounds(
							scene.Objects(),
							m_Renderer.ObjectLocalBounds());

					const float framingAspectRatio = aspectRatio;

                    SceneCameraFraming framing = scene.Objects().empty()
                        ? CreateEmptySceneCameraFraming(framingAspectRatio)
                        : CreateSceneCameraFraming(framingBounds, framingAspectRatio);

					if (const auto& savedCamera = scene.SavedCamera())
					{
                        framing.camera = CreateSceneCamera(*savedCamera);
					}

					camera = std::move(framing.camera);
					cameraController = std::move(framing.controller);
                    if (scene.CameraMovementSpeed())
                        cameraController.SetMovementSpeed(*scene.CameraMovementSpeed());

					viewportNavigationActive = false;

					m_Window.SetCursorMode(
						Platform::CursorMode::Normal);

					// Exclude synchronous scene/model loading from navigation time.
					static_cast<void>(timer.Restart());
				}
				catch (const std::exception& exception)
				{
					sceneFileMessage =
						"Scene changed, but camera framing failed: " +
						std::string{ exception.what() };

					Core::LogError(sceneFileMessage);
				}
			}

			const auto frameDelta = timer.Restart();

			const float deltaSeconds =
				std::chrono::duration<float>{ frameDelta }.count();

			const auto fixedStepResult =
				scheduler.AddTime(frameDelta);

			for (std::uint32_t step = 0;
				step < fixedStepResult.stepCount;
				++step)
			{
				// Perform fixed-step update logic here.
			}

			if (const auto diagnosticsSnapshot =
				runtimeDiagnostics.AddFrame(
					frameDelta,
					fixedStepResult))
			{
				latestDiagnosticsSnapshot =
					*diagnosticsSnapshot;

				const double droppedMilliseconds =
					std::chrono::duration<double, std::milli>{
						diagnosticsSnapshot->droppedSimulationTime
				}.count();

				ForgeSim::Core::LogInfo(
					"FPS: " +
					std::to_string(
						diagnosticsSnapshot
						->frameStatistics
						.framesPerSecond) +

					", Average Frame Time: " +
					std::to_string(
						diagnosticsSnapshot
						->frameStatistics
						.averageFrameTimeMilliseconds) +
					" ms" +

					" | Fixed updates: " +
					std::to_string(
						diagnosticsSnapshot
						->fixedUpdateCount) +

					" | Catch-up limit events: " +
					std::to_string(
						diagnosticsSnapshot
						->catchUpLimitedFrameCount) +

					" | Dropped simulation time: " +
					std::to_string(droppedMilliseconds) +
					" ms");
			}

			if (viewportNavigationActive &&
				!changePending &&
				!transformEditor.IsActive() &&
				!leftMouseDown)
			{
                if (cameraController.AdjustMovementSpeed(input.GetScrollDelta()))
                    static_cast<void>(scene.TrySetCameraMovementSpeed(cameraController.MovementSpeed()));
				cameraController.Update(
					camera,
					input,
					deltaSeconds);
			}

			const glm::mat4 view =
				camera.ViewMatrix();

			const glm::mat4 projection =
				camera.ProjectionMatrix(aspectRatio);

			ClearMissingSceneSelection(
				selection,
				scene);

            // Keep the Inspector selection when hiding an object, but omit its gizmo.
            SceneSelection gizmoSelection = selection;
            if (const auto id = selection.SelectedObjectId(); id && !scene.IsObjectVisible(*id))
                gizmoSelection.Clear();
            if (const auto id = selection.SelectedLightId(); id && !scene.IsLightVisible(*id))
                gizmoSelection.Clear();

			transformEditor.Update(
				SceneTransformEditorContext{
					.scene = scene,
					.selection = gizmoSelection,
					.view = view,
					.projection = projection,
					.viewportPosition = viewport.position,
					.viewportSize = viewport.size,
					.viewportHovered = viewport.hovered && !changePending,
					.operation = editorUI.operation,
					.space = editorUI.space,
					.showLights = m_DebugUI.ShowLights(),				
					.navigationActive = changePending|| rightMouseDown
				});

            bool sceneEntitiesChanged = false;
			if (!changePending && !sceneFilePanel.ConsoleWantsKeyboard() && (editorUI.undoRequested || toolbarRequest.undo))
			{
				sceneEntitiesChanged = transformEditor.Undo(scene, &selection);
			}
			else if (!changePending && !sceneFilePanel.ConsoleWantsKeyboard() && (editorUI.redoRequested || toolbarRequest.redo))
			{
				sceneEntitiesChanged = transformEditor.Redo(scene, &selection);
			}

            else if (!changePending && !sceneFilePanel.ConsoleWantsKeyboard() && !rightMouseDown &&
                (viewport.duplicateSelectionRequested || hierarchyPanel.DuplicateSelectionRequested()))
            {
                sceneEntitiesChanged = transformEditor.DuplicateSelection(scene, selection);
                if (sceneEntitiesChanged) sceneFileMessage = "Selection duplicated at the same position. Use Move to separate the copy.";
            }
            else if (fileRequest.createCube && !changePending && !rightMouseDown &&
                !sceneFilePanel.ConsoleWantsKeyboard())
            {
                try
                {
                    // Prepare resources before mutating the active scene/history.
                    SandboxScene prepared = scene;
                    static_cast<void>(prepared.AddModelObject("Cube", std::string{BuiltinCubeReference}));
                    m_Renderer.LoadSceneModels(m_EditorSession->ProjectRoot(), prepared.Objects(),
                        ResolveSceneEnvironment(scene.RenderingSettings(), m_EditorSession->ProjectRoot(),
                            BundledAssetDirectory() / "environments"));
                    const auto position = camera.Position() + camera.ForwardDirection() * 3.0f;
                    sceneEntitiesChanged = transformEditor.CreateCube(scene, selection, position);
                    if (sceneEntitiesChanged) sceneFileMessage = "Cube created. Undo removes it.";
                }
                catch (const std::exception& error)
                {
                    sceneFileMessage = std::string{"Could not create cube: "} + error.what();
                }
            }
            else if (fileRequest.createLight && !changePending && !rightMouseDown &&
                !sceneFilePanel.ConsoleWantsKeyboard())
            {
                const auto worldBounds = scene.Objects().empty() ? emptySceneBounds :
                    CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());
                const float radius = glm::length(worldBounds.maximum * 0.5f - worldBounds.minimum * 0.5f);
                const float distance = std::clamp(radius * 0.25f, 0.01f, 10.0f);
                const auto direction = camera.ForwardDirection();
                const auto position = camera.Position() + direction * distance;
                sceneEntitiesChanged = transformEditor.CreateLight(scene, selection, *fileRequest.createLight,
                    position, direction, std::max(distance * 4.0f, 0.1f));
                if (sceneEntitiesChanged) sceneFileMessage = "Light created. Undo removes it.";
            }
            else if (!changePending && !sceneFilePanel.ConsoleWantsKeyboard() && !rightMouseDown &&
                (viewport.deleteSelectionRequested || hierarchyPanel.DeleteSelectionRequested()))
            {
                sceneEntitiesChanged = transformEditor.DeleteSelection(scene, selection);
                if (sceneEntitiesChanged) sceneFileMessage = "Selection deleted. Undo restores it.";
            }
            if (sceneEntitiesChanged)
            {
                ClearMissingSceneSelection(selection, scene);
                inspectorPanel.Reset();
                lightPickTargets.clear();
                lightDebugBounds = scene.Objects().empty() ? emptyLightDebugBounds :
                    CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());
            }

			const bool pickingSuppressed =
				!viewport.hovered ||
				changePending ||
				viewportNavigationActive ||
				transformEditor.SuppressesPicking();

            // Feed the same visible subset to picking and all render passes,
            // including object bounds, highlights, and shadows.
            visibleObjects.clear();
            for (const auto& object : scene.Objects())
                if (scene.IsObjectVisible(object.id)) visibleObjects.push_back(object);

            std::erase_if(lightPickTargets, [&](const LightPickTarget& target)
                { return !scene.IsLightVisible(target.id); });

			selectionController.Update(
				input,
				viewport.cursorPixels,
				viewport.framebufferExtent,
				view,
				projection,
				m_Renderer.ObjectLocalBounds(),
				visibleObjects,
				lightPickTargets,
				selection,
				pickingSuppressed);

			const SandboxRenderSettings renderSettings{
				.framebufferWidth =
					static_cast<std::uint32_t>(
						viewport.framebufferExtent.width),

				.framebufferHeight =
					static_cast<std::uint32_t>(
						viewport.framebufferExtent.height),

				.backgroundColor =
					scene.RenderingSettings().backgroundColor,

				.exposure = scene.RenderingSettings().exposure,

				.environmentIntensity =
					scene.RenderingSettings().environmentIntensity,

				.showEnvironmentBackground =
					scene.RenderingSettings().showEnvironmentBackground,

				.showGrid = m_DebugUI.ShowGrid(),

				.showObjectBounds =
					m_DebugUI.ShowObjectBounds(),

				.showLights = m_DebugUI.ShowLights(),
                .showDirectionalLightGuide = scene.HasDirectionalLight() && scene.IsLightVisible(scene.MainDirectionalLight().id),

				.directionalShadows = scene.RenderingSettings().directionalShadows,
                .fxaa = m_DebugUI.Fxaa(),

				.lightDebugBounds = lightDebugBounds,

				.selectedLightId = selection.SelectedLightId(),

				.selectionHighlight = {
					.enabled =
						m_DebugUI.SelectionHighlightEnabled(),

					.color =
						m_DebugUI.SelectionHighlightColor(),

					.strength =
						m_DebugUI.SelectionHighlightStrength()
					},

				.renderToTexture = true
			};

            DirectionalLight visibleDirectional = scene.MainDirectionalLight();
            visibleDirectional.enabled &= scene.HasDirectionalLight() && scene.IsLightVisible(visibleDirectional.id);
            visiblePointLights.clear();
            for (const auto& light : scene.PointLights())
                if (scene.IsLightVisible(light.id)) visiblePointLights.push_back(light);
            visibleSpotLights.clear();
            for (const auto& light : scene.SpotLights())
                if (scene.IsLightVisible(light.id)) visibleSpotLights.push_back(light);

			m_Renderer.Render(
				visibleObjects,
				selection.SelectedObjectId(),
				view,
				projection,
				camera.Position(),
				SceneLightingView{
					.directionalLight = visibleDirectional,
					.pointLights = visiblePointLights,
					.spotLights = visibleSpotLights
				},
				renderSettings);

			sceneViewportPanel.End();

            const auto focusSelection = [&]
            {
                const auto focusBounds = SelectionFocusBounds(scene.Objects(),
                    SceneLightingView{scene.MainDirectionalLight(), scene.PointLights(), scene.SpotLights()},
                    selection, m_Renderer.ObjectLocalBounds());
                if (!focusBounds) return;
                const auto sceneBounds = CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());
                const auto focused = CreateSelectionFocusCamera(*focusBounds, sceneBounds, camera, aspectRatio);
                cameraRestoreRequest = CaptureSceneCamera(focused);
                // Focusing changes the viewpoint, never the fly-speed setting or scene data.
            };
            if ((viewport.focusSelectionRequested || hierarchyPanel.FocusSelectionRequested()) &&
                !changePending && !transformEditor.IsActive() && !rightMouseDown &&
                !sceneFilePanel.ConsoleWantsKeyboard() && viewport.IsDrawable())
                focusSelection();

			m_DebugUI.Draw(
				SandboxDebugUIContext{
					.diagnosticsSnapshot =
						latestDiagnosticsSnapshot 
							? &*latestDiagnosticsSnapshot 
							: nullptr,
					.camera = camera,
                    .cameraController = cameraController,
				.scene = scene,
				.cameraEditingDisabled =
                    changePending || transformEditor.IsActive() || rightMouseDown || !viewport.IsDrawable(),
                .referenceViewport = viewport.IsDrawable()
                    ? std::optional<SceneCaptureExtent>{ SceneCaptureExtent{
                        static_cast<std::uint32_t>(viewport.framebufferExtent.width),
                        static_cast<std::uint32_t>(viewport.framebufferExtent.height) } }
                    : std::nullopt,
                .cameraRestoreRequest = cameraRestoreRequest,
                .canFocusSelection = !selection.Empty(),
                .focusSelection = focusSelection,
                .frameScene = [&]
                {
                    auto framing = scene.Objects().empty() ? CreateEmptySceneCameraFraming(aspectRatio) :
                        CreateSceneCameraFraming(CalculateSceneBounds(
                            scene.Objects(), m_Renderer.ObjectLocalBounds()), aspectRatio);
                    cameraRestoreRequest = CaptureSceneCamera(framing.camera);
                    cameraController = std::move(framing.controller);
                    if (scene.CameraMovementSpeed())
                        cameraController.SetMovementSpeed(*scene.CameraMovementSpeed());
                },

                .applyEnvironment = [&](const SceneRenderingSettings& settings)
                {
                    // Prepare authored state before touching GPU resources.
                    SandboxScene replacement = scene;
                    if (const auto error = replacement.TrySetRenderingSettings(settings))
                        throw std::invalid_argument(std::string{ error->message });
                    const auto environment = ResolveSceneEnvironment(settings,
                        m_EditorSession->ProjectRoot(), BundledAssetDirectory() / "environments");
                    m_Renderer.LoadSceneResources(std::nullopt, environment);
                    static_assert(std::is_nothrow_move_assignable_v<SandboxScene>);
                    scene = std::move(replacement);
                    static_cast<void>(timer.Restart());
                },
				}
			);

			inspectorPanel.Draw(
				SceneInspectorPanelContext{
					.scene = scene,
					.selection = selection,
					.modelBounds = editorBounds,
					.editingDisabled =
						changePending ||
						transformEditor.IsActive() ||
						rightMouseDown,
                    .transformEditor = &transformEditor,
                    .materials = selection.SelectedObjectId() && scene.FindObject(*selection.SelectedObjectId())
                        ? m_Renderer.InspectModelMaterials(scene.FindObject(*selection.SelectedObjectId())->modelPath)
                        : ModelMaterialInspection{}
				});

			bool openAfterFrame = false;
            std::optional<std::filesystem::path> importAfterFrame;
			if (fileRequest.action != SceneFileAction::None)
			{
				try
				{
					bool completePendingAction = false;

					switch (fileRequest.action)
					{
                    case SceneFileAction::ImportModel:
                        if (transformEditor.IsActive() || changePending || rightMouseDown) break;
                        importAfterFrame = Platform::ShowFileDialog(m_Window,
                            Platform::FileDialogMode::Open,
                            Platform::FileDialogSpecification{
                                .defaultDirectory = m_EditorSession->ProjectRoot(),
                                .title = L"Import model into scene",
                                .filterName = L"glTF models",
                                .filterPattern = L"*.glb;*.gltf",
                                .purpose = Platform::FileDialogPurpose::ModelImport
                            });
                        static_cast<void>(timer.Restart());
                        break;
					case SceneFileAction::New:
						if (transformEditor.IsActive() || changePending)
						{
							break;
						}

						sceneFileMessage.clear();
						pendingAction = PendingSceneAction::New;

						if (!m_EditorSession->HasUnsavedChanges())
						{
							completePendingAction = true;
						}
						break;

					case SceneFileAction::BrowseExternalSample:
					{
						if (transformEditor.IsActive() || changePending)
							break;
						const auto path = ChooseScenePath(false);
						static_cast<void>(timer.Restart());
						if (path)
							sceneFilePanel.SetExternalSamplePath(*path);
						break;
					}

					case SceneFileAction::OpenExternalSample:
					case SceneFileAction::OpenSample:
					case SceneFileAction::Open:
					{
						if (transformEditor.IsActive() || changePending)
						{
							break;
						}

						const bool bundledSample = fileRequest.action == SceneFileAction::OpenSample;
						const bool externalSample = fileRequest.action == SceneFileAction::OpenExternalSample;
						const bool sample = bundledSample || externalSample;
						if (bundledSample && !IsBundledSample(fileRequest.path))
							throw std::invalid_argument("Unknown bundled sample.");
						const auto path = bundledSample
							? std::optional<std::filesystem::path>{BundledAssetDirectory() / "scenes" / fileRequest.path}
							: externalSample
								? std::optional<std::filesystem::path>{SamplePathFromUtf8(fileRequest.path)}
								: ChooseScenePath(false);
						static_cast<void>(timer.Restart());

						if (!path)
						{
							break;
						}

						pendingOpenPath = *path;
						pendingSample = sample;
                        pendingTexturePolicy = externalSample
                            ? std::optional{fileRequest.allowTextureFallbacks
                                ? Assets::GltfTexturePolicy::SkipUnusableTextures
                                : Assets::GltfTexturePolicy::Strict}
                            : std::nullopt;
						pendingProjectRoot = bundledSample
							? BundledAssetDirectory().parent_path()
							: ResolveSceneAssetRoot(*path, sceneOpenRoot.Override(),
                                    BundledAssetDirectory().parent_path()).root;
						sceneFileMessage.clear();
						pendingAction = PendingSceneAction::Open;

						if (!m_EditorSession->HasUnsavedChanges())
						{
							completePendingAction = true;
						}

						break;
					}

					case SceneFileAction::Save:
					{
						if (transformEditor.IsActive() || changePending)
						{
							break;
						}

						if (m_EditorSession->ScenePath())
						{
							m_EditorSession->Save();
						}
						else
						{
							const auto path = ChooseScenePath(true);
							static_cast<void>(timer.Restart());

							if (!path)
							{
								break;
							}

							m_EditorSession->SaveAs(*path);
						}

						sceneFileMessage = "Scene saved.";
                        Core::LogInfo("Scene saved: " + m_EditorSession->DisplayName());
						break;
					}

					case SceneFileAction::SaveAs:
					{
						if (transformEditor.IsActive() || changePending)
						{
							break;
						}

						const auto path = ChooseScenePath(true);
						static_cast<void>(timer.Restart());

						if (!path)
						{
							break;
						}

						m_EditorSession->SaveAs(*path);
						sceneFileMessage = "Scene saved.";
                        Core::LogInfo("Scene saved: " + m_EditorSession->DisplayName());
						break;
					}

					case SceneFileAction::SaveAndContinue:
					{
						if (pendingAction == PendingSceneAction::None)
						{
							break;
						}

						if (transformEditor.IsActive())
						{
							sceneFileMessage =
								"Finish the transform edit before saving.";
							break;
						}

						if (m_EditorSession->ScenePath())
						{
							m_EditorSession->Save();
						}
						else
						{
							const auto path = ChooseScenePath(true);
							static_cast<void>(timer.Restart());

							if (!path)
							{
								// Canceling the native picker leaves the
								// unsaved-changes confirmation pending.
								break;
							}

							m_EditorSession->SaveAs(*path);
						}

                        Core::LogInfo("Scene saved: " + m_EditorSession->DisplayName());
						completePendingAction = true;
						break;
					}

					case SceneFileAction::DiscardAndContinue:
						if (pendingAction == PendingSceneAction::Close)
						{
							m_SaveOnExit = false;
						}

						completePendingAction =
							pendingAction != PendingSceneAction::None;
						break;

                    case SceneFileAction::LocateMissingModel:
                    {
                        if (!recoveringAssets || missingModelReference.empty()) break;
                        const auto model = Platform::ShowFileDialog(m_Window,
                            Platform::FileDialogMode::Open,
                            Platform::FileDialogSpecification{
                                .defaultDirectory = pendingOpenPath.parent_path(),
                                .suggestedFileName = {},
                                .title = L"Locate missing model",
                                .filterName = L"glTF model",
                                .filterPattern = L"*.glb;*.gltf",
                                .purpose = Platform::FileDialogPurpose::ModelRecovery
                            });
                        static_cast<void>(timer.Restart());
                        if (!model) break;
                        const auto root = InferSceneAssetRoot(*model, missingModelReference);
                        if (!root)
                        {
                            assetRecoveryMessage += "\n\nThe selected file does not match the scene's model path: " +
                                missingModelReference +
                                "\nRestore the original folder names, or update the scene's model reference. Selecting a file does not rename or remap assets.";
                            break;
                        }
                        pendingProjectRoot = *root;
                        recoveringAssets = false;
                        assetRecoveryMessage.clear();
                        completePendingAction = true;
                        break;
                    }
                    case SceneFileAction::LocateMissingAssets:
                    {
                        if (!recoveringAssets) break;
                        const auto folder = Platform::ShowFileDialog(m_Window,
                            Platform::FileDialogMode::SelectFolder,
                            Platform::FileDialogSpecification{
                                .defaultDirectory = pendingOpenPath.parent_path(),
                                .suggestedFileName = {},
                                .title = L"Choose asset folder (contains the referenced paths)"
                            });
                        static_cast<void>(timer.Restart());
                        if (!folder) break;
                        pendingProjectRoot = *folder;
                        recoveringAssets = false;
                        assetRecoveryMessage.clear();
                        completePendingAction = true;
                        break;
                    }
                    case SceneFileAction::RetryMissingAssets:
                        if (recoveringAssets)
                        {
                            recoveringAssets = false;
                            assetRecoveryMessage.clear();
                            completePendingAction = true;
                        }
                        break;

					case SceneFileAction::CancelPending:
                        recoveringAssets = false;
                        assetRecoveryMessage.clear();
						pendingAction = PendingSceneAction::None;
						pendingOpenPath.clear();
						pendingProjectRoot.reset();
						pendingSample = false;
                    pendingTexturePolicy.reset();
						sceneFileMessage.clear();
						break;

					case SceneFileAction::None:
						break;
					}

					if (completePendingAction)
					{
						if (pendingAction == PendingSceneAction::Close)
						{
							exitRequested = true;
						}
						else if (
							pendingAction == PendingSceneAction::New)
						{
							EditorSession replacement =
								m_EditorSession->PrepareNew();

                            m_Renderer.LoadSceneModels(replacement.ProjectRoot(), replacement.Scene().Objects(),
                                ResolveSceneEnvironment(replacement.Scene().RenderingSettings(),
                                    replacement.ProjectRoot(), BundledAssetDirectory() / "environments"), {}, std::nullopt, false);

							// Keep the EditorSession and its SandboxScene
							// at their existing addresses.
							*m_EditorSession = std::move(replacement);
							sceneNavigationResetRequested = true;
                            m_DebugUI.ResetSceneControls();
                            cameraRestoreRequest.reset();

							lightDebugBounds =
								scene.Objects().empty()
								? emptyLightDebugBounds
								: CalculateSceneBounds(
									scene.Objects(),
									m_Renderer.ObjectLocalBounds());

							selection.Clear();
							transformEditor.ClearHistory();
							inspectorPanel.Reset();
							lightPickTargets.clear();
							cameraFramingRequested = true;

							pendingAction = PendingSceneAction::None;
							sceneFileMessage = "New scene created.";
                            Core::LogInfo(sceneFileMessage);
						}
						else if (pendingAction == PendingSceneAction::Open)
						{
							openAfterFrame = true;
						}
					}
				}
				catch (const std::exception& exception)
				{
					sceneFileMessage = exception.what();
					Core::LogError(sceneFileMessage);
				}
			}

			m_Renderer.ClearWindow(
				static_cast<std::uint32_t>(framebufferExtent.width),
				static_cast<std::uint32_t>(framebufferExtent.height));

			m_DebugUI.EndFrame();

			m_Window.SwapBuffers();
			if (startupStart && !firstFramePresented && framebufferExtent.width > 0 && framebufferExtent.height > 0)
			{
				firstFramePresented = true;
				const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - *startupStart).count();
				Core::LogInfo("Startup through first editor buffer swap: " + std::to_string(elapsed) + " ms");
			}

            if (importAfterFrame)
            {
                try
                {
                    EditorSession replacement = m_EditorSession->PrepareImportModel(*importAfterFrame);
                    auto& imported = replacement.Scene().Objects().back();
                    const auto drawProgress = [this](const Assets::GltfLoadProgress& progress)
                    { PresentLoadingFrame(m_Window, m_DebugUI, progress); };
                    m_Renderer.LoadSceneModels(replacement.ProjectRoot(), replacement.Scene().Objects(),
                        ResolveSceneEnvironment(replacement.Scene().RenderingSettings(), replacement.ProjectRoot(),
                            BundledAssetDirectory() / "environments"), drawProgress);
                    const auto previous = scene.Objects();
                    if (!previous.empty())
                    {
                        const auto existing = CalculateSceneBounds(previous, m_Renderer.ObjectLocalBounds());
                        const auto& importedBounds = *imported.localBounds;
                        const float gap = std::max(existing.maximum.x - existing.minimum.x,
                            importedBounds.maximum.x - importedBounds.minimum.x) * 0.1f;
                        imported.transform.position.x = existing.maximum.x + gap - importedBounds.minimum.x;
                    }
                    const auto id = imported.id;
                    *m_EditorSession = std::move(replacement);
                    sceneNavigationResetRequested = true;
                    selection.SelectObject(id);
                    inspectorPanel.Reset();
                    lightPickTargets.clear();
                    lightDebugBounds = CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());
                    const auto framing = CreateSceneCameraFraming(lightDebugBounds, aspectRatio);
                    cameraRestoreRequest = CaptureSceneCamera(framing.camera);
                    cameraFramingRequested = false;
                    sceneFileMessage = "Model imported. Save the scene to keep it.";
                }
                catch (const std::exception& error)
                {
                    sceneFileMessage = "Model import failed: " + std::string(error.what());
                    Core::LogError(sceneFileMessage);
                }
                static_cast<void>(timer.Restart());
            }

			// The editor frame must end before beginning a loading frame.
			if (openAfterFrame)
			{
                const bool logLoadTimings = m_Renderer.LoadingDiagnostics();
                const auto loadStart = std::chrono::steady_clock::now();
                const auto logTiming = [&](std::string_view label, auto start)
                {
                    if (logLoadTimings)
                    {
                        const double milliseconds = std::chrono::duration<double, std::milli>(
                            std::chrono::steady_clock::now() - start).count();
                        Core::LogInfo(std::string(label) + ": " + std::to_string(milliseconds) + " ms");
                    }
                };
				try
				{
					const auto drawProgress = [this](const Assets::GltfLoadProgress& progress)
					{
						PresentLoadingFrame(m_Window, m_DebugUI, progress);
					};
					drawProgress({"Reading scene document", Assets::GltfProgressPhase::Started,
						0, std::nullopt, std::chrono::milliseconds{0}});
					// Read after Save and Continue, so opening the same
					// file uses the version that was just saved.
                    const auto documentStart = std::chrono::steady_clock::now();
					EditorSession replacement =
						pendingSample
							? m_EditorSession->PrepareSample(pendingOpenPath, pendingProjectRoot.value_or(sceneOpenRoot.Path()))
							: m_EditorSession->PrepareOpen(pendingOpenPath, pendingProjectRoot);
                    logTiming("Scene document loaded", documentStart);
                    const auto resourcesStart = std::chrono::steady_clock::now();

                    m_Renderer.LoadSceneModels(replacement.ProjectRoot(), replacement.Scene().Objects(),
                        ResolveSceneEnvironment(replacement.Scene().RenderingSettings(),
							replacement.ProjectRoot(), BundledAssetDirectory() / "environments"), drawProgress, pendingTexturePolicy, false);
                    logTiming("Scene resources ready", resourcesStart);

					// Empty scenes render no objects and can retain
					// the renderer's existing unused model resources.
					*m_EditorSession = std::move(replacement);
					sceneNavigationResetRequested = true;
					m_DebugUI.ResetSceneControls();
					cameraRestoreRequest.reset();

					lightDebugBounds = scene.Objects().empty()
						? emptyLightDebugBounds
						: CalculateSceneBounds(scene.Objects(), m_Renderer.ObjectLocalBounds());

					selection.Clear();
					transformEditor.ClearHistory();
					inspectorPanel.Reset();
					lightPickTargets.clear();
					cameraFramingRequested = true;

					pendingAction = PendingSceneAction::None;
					pendingOpenPath.clear();
					pendingProjectRoot.reset();
					pendingSample = false;
                    pendingTexturePolicy.reset();
					sceneFileMessage = "Scene opened.";
                    Core::LogInfo("Scene loaded: " + m_EditorSession->DisplayName());
                    logTiming("Scene open completed", loadStart);
				}
                catch (const MissingSceneAssets& missing)
                {
                    // Retain the requested scene/root and load options for retry.
                    // No replacement session or renderer resources were committed.
                    recoveringAssets = true;
                    assetRecoveryMessage = DescribeMissingSceneAssets(pendingOpenPath, missing);
                    missingModelReference.clear();
                    for (const auto& asset : missing.assets)
                        if (asset.field != "rendering.environment.asset")
                        {
                            missingModelReference = asset.reference;
                            break;
                        }
                    sceneFileMessage = "Scene assets are missing. Locate their folder or cancel.";
                    Core::LogWarning(sceneFileMessage);
                }
				catch (const std::exception& exception)
				{
					pendingAction = PendingSceneAction::None;
					pendingOpenPath.clear();
					pendingProjectRoot.reset();
					pendingSample = false;
                    pendingTexturePolicy.reset();
					sceneFileMessage = exception.what();
					Core::LogError(sceneFileMessage);
                    logTiming("Scene open failed after", loadStart);
				}
			}
		}
	}

	void SandboxApplication::SaveScene(
		const std::filesystem::path& path)
	{
		m_EditorSession->SaveAs(
			std::filesystem::absolute(path));
	}

	std::optional<std::filesystem::path>
		SandboxApplication::ChooseScenePath(bool saving)
	{
		Platform::FileDialogSpecification specification;

		specification.defaultDirectory =
			m_EditorSession->DefaultSceneDirectory();

		specification.suggestedFileName =
			m_EditorSession->ScenePath()
			? m_EditorSession->ScenePath()->filename()
			: std::filesystem::path{ "Untitled.scene.json" };

		specification.title =
			saving ? L"Save ForgeSim Scene" : L"Open ForgeSim Scene";

		specification.filterName = L"ForgeSim scene files";
		specification.filterPattern = L"*.scene.json;*.json";
		specification.defaultExtension = L"scene.json";

		m_Window.SetCursorMode(Platform::CursorMode::Normal);

		return Platform::ShowFileDialog(
			m_Window,
			saving
			? Platform::FileDialogMode::Save
			: Platform::FileDialogMode::Open,
			specification);
	}
}
