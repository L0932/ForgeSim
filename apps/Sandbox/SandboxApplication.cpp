#include "FreeCameraController.hpp"
#include "LightDebugLayout.hpp"
#include "LightPicking.hpp"
#include "SandboxApplication.hpp"
#include "SceneCameraFraming.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneFilePanel.hpp"
#include "SceneHierarchyPanel.hpp"
#include "SceneInspectorPanel.hpp"
#include "SceneTransformEditor.hpp"
#include "SceneViewportPanel.hpp"
#include "SceneSelection.hpp"
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
	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_Renderer(spec.rendererSpec)
		, m_DebugUI(m_Window)
		, m_LoadedScene(spec.initialScene.has_value())
	{
		const std::filesystem::path projectRoot =
			std::filesystem::weakly_canonical(
				std::filesystem::absolute(
					spec.projectRoot.value_or(
						std::filesystem::path{
							FORGESIM_SANDBOX_ASSET_DIRECTORY
						}.parent_path())));

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
			spec.scenePath);

		Core::LogInfo(
			"Scene project root: " + projectRoot.string());
	}

	void SandboxApplication::Run()
	{
		if (m_RunAttempted)
		{
			throw std::logic_error(
				"Run() can only be called once per application instance.");
		}

		m_RunAttempted = true;
		
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

		SceneCameraFraming initialFraming =
			CreateSceneCameraFraming(
				scene.Objects().empty() ? emptySceneBounds : bounds,
				initialAspectRatio);

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

		SceneFilePanel sceneFilePanel;
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

		// Committing a prepared session must not fail after the model changes.
		static_assert(
			std::is_nothrow_move_assignable_v<EditorSession>);

		bool exitRequested = false;
		bool cameraFramingRequested = true;
		bool viewportNavigationActive = false;

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
					CalculateLightDebugLayout(editorBounds);

				lightPickTargets.push_back(
					LightPickTarget{
						.id = scene.MainDirectionalLight().id,
						.bounds = {
							.center = scene.MainDirectionalLight().position,
							.radius = layout.pickRadius
						}
					});

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
				m_EditorSession->ScenePath().has_value());

			m_DebugUI.BeginWorkspace();

			const TransformEditorUIState editorUI =
				m_DebugUI.DrawTransformEditorControls(
					transformEditor.IsActive(),
					changePending||
					input.GetMouseButtonState(
						Platform::MouseButton::Right).held);

			runtimeDiagnostics.SetEnabled(
				m_DebugUI.RuntimeDiagnosticsEnabled());

			if (!runtimeDiagnostics.IsEnabled())
			{
				latestDiagnosticsSnapshot.reset();
			}

			std::string scenePathText;
			std::string sceneDisplayName = scene.Name();

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
						.confirmationPending = changePending,
						.pendingActionText =
							pendingAction == PendingSceneAction::New
								? "creating a new scene"
								: pendingAction == PendingSceneAction::Open
									? "opening another scene"
									: "closing"
					});


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

			const SceneViewportState& viewport =
				sceneViewportPanel.Begin(framebufferScale);

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

			const bool leftMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Left).held;

			const bool rightMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Right).held;

			const bool previouslyNavigating = viewportNavigationActive;

			if (!rightMouseDown ||
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

					SceneCameraFraming framing =
						CreateSceneCameraFraming(
							framingBounds,
							framingAspectRatio);

					camera = std::move(framing.camera);
					cameraController = std::move(framing.controller);

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

			transformEditor.Update(
				SceneTransformEditorContext{
					.scene = scene,
					.selection = selection,
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

			if (!changePending&& editorUI.undoRequested)
			{
				static_cast<void>(transformEditor.Undo(scene));
			}
			else if (!changePending&& editorUI.redoRequested)
			{
				static_cast<void>(transformEditor.Redo(scene));
			}

			const bool pickingSuppressed =
				!viewport.hovered ||
				changePending ||
				viewportNavigationActive ||
				transformEditor.SuppressesPicking();

			selectionController.Update(
				input,
				viewport.cursorPixels,
				viewport.framebufferExtent,
				view,
				projection,
				m_Renderer.ObjectLocalBounds(),
				scene.Objects(),
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
					m_DebugUI.BackgroundColor(),

				.exposure = m_DebugUI.Exposure(),
				
				.environmentIntensity =
					m_DebugUI.EnvironmentIntensity(),

				.showEnvironmentBackground =
					m_DebugUI.ShowEnvironmentBackground(),

				.showGrid = m_DebugUI.ShowGrid(),

				.showObjectBounds =
					m_DebugUI.ShowObjectBounds(),

				.showLights = m_DebugUI.ShowLights(),

				.lightDebugBounds = editorBounds,

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

			m_Renderer.Render(
				scene.Objects(),
				selection.SelectedObjectId(),
				view,
				projection,
				camera.Position(),
				SceneLightingView{
					.directionalLight = scene.MainDirectionalLight(),
					.pointLights = scene.PointLights(),
					.spotLights = scene.SpotLights()
				},
				renderSettings);

			sceneViewportPanel.End();

			m_DebugUI.Draw(
				SandboxDebugUIContext{
					.diagnosticsSnapshot =
						latestDiagnosticsSnapshot 
							? &*latestDiagnosticsSnapshot 
							: nullptr,
					.camera = camera,
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
						rightMouseDown
				});

			if (fileRequest.action != SceneFileAction::None)
			{
				try
				{
					bool completePendingAction = false;

					switch (fileRequest.action)
					{
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

					case SceneFileAction::Open:
					{
						if (transformEditor.IsActive() || changePending)
						{
							break;
						}

						const auto path = ChooseScenePath(false);
						static_cast<void>(timer.Restart());

						if (!path)
						{
							break;
						}

						pendingOpenPath = *path;
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

					case SceneFileAction::CancelPending:
						pendingAction = PendingSceneAction::None;
						pendingOpenPath.clear();
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

							// Keep the EditorSession and its SandboxScene
							// at their existing addresses.
							*m_EditorSession = std::move(replacement);

							selection.Clear();
							transformEditor.ClearHistory();
							inspectorPanel.Reset();
							lightPickTargets.clear();
							cameraFramingRequested = true;

							pendingAction = PendingSceneAction::None;
							sceneFileMessage = "New scene created.";
						}
						else if (pendingAction == PendingSceneAction::Open)
						{
							try
							{
								// Read after Save and Continue, so opening the same
								// file uses the version that was just saved.
								EditorSession replacement =
									m_EditorSession->PrepareOpen(pendingOpenPath);

								const auto modelPath =
									replacement.SharedModelPath();

								if (modelPath)
								{
									// Import and GPU upload finish before committing
									// the replacement session.
									m_Renderer.LoadObjectModel(*modelPath);
								}

								// Empty scenes render no objects and can retain
								// the renderer's existing unused model resources.
								*m_EditorSession = std::move(replacement);

								selection.Clear();
								transformEditor.ClearHistory();
								inspectorPanel.Reset();
								lightPickTargets.clear();
								cameraFramingRequested = true;

								pendingAction = PendingSceneAction::None;
								pendingOpenPath.clear();
								sceneFileMessage = "Scene opened.";
							}
							catch (...)
							{
								// End the pending operation. The outer handler
								// displays the loading error.
								pendingAction = PendingSceneAction::None;
								pendingOpenPath.clear();
								throw;
							}
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
			m_EditorSession->ProjectRoot() / "scenes";

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