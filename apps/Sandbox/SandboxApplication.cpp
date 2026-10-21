#include "FreeCameraController.hpp"
#include "LightDebugLayout.hpp"
#include "LightPicking.hpp"
#include "SandboxApplication.hpp"
#include "SceneDocumentIO.hpp"
#include "SceneTransformEditor.hpp"
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
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/RuntimeDiagnostics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

namespace ForgeSim::Sandbox
{
	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_Scene(
			spec.initialScene
				? SandboxScene( *spec.initialScene )
				: SandboxScene{})
		, m_Renderer(spec.rendererSpec)
		, m_DebugUI(m_Window)
		, m_ProjectRoot(
			std::filesystem::weakly_canonical(
				std::filesystem::absolute(
					spec.projectRoot.value_or(
					std::filesystem::path{
						FORGESIM_SANDBOX_ASSET_DIRECTORY
					}.parent_path()))))
		, m_LoadedScene(spec.initialScene.has_value())
	{

		if (!m_LoadedScene)
		{
			SandboxObject& object = m_Scene.Objects().front();

			object.name =
				m_Renderer.ObjectModelPath().stem().string();

			const std::filesystem::path relativeModelPath =
				m_Renderer.ObjectModelPath()
				.lexically_relative(m_ProjectRoot);

			const std::u8string reference =
				relativeModelPath.generic_u8string();

			object.modelPath.assign(
				reference.begin(),
				reference.end());

			m_Scene.MainDirectionalLight().position =
				CalculateLightDebugLayout(
					m_Renderer.ObjectLocalBounds()).directionalAnchor;
		}

		Core::LogInfo(
			"Scene project root: " + m_ProjectRoot.string());

		for (const SandboxObject& object : m_Scene.Objects())
		{
			Core::LogInfo(
				"Scene model reference: " + object.modelPath);
		}
	}

	void SandboxApplication::SaveScene(
		const std::filesystem::path& path) const
	{
		SaveSceneDocument(
			m_Scene.ToDocument(),
			path,
			m_ProjectRoot);
	}

	void SandboxApplication::Run()
	{
		if (m_RunAttempted)
		{
			throw std::logic_error(
				"Run() can only be called once per application instance.");
		}

		m_RunAttempted = true;

		m_Window.Show();

		const Renderer::AxisAlignedBoundingBox bounds =
			CalculateSceneBounds(
				m_Scene.Objects(),
				m_Renderer.ObjectLocalBounds());

		const glm::vec3 modelCenter =
			(bounds.minimum + bounds.maximum) * 0.5f;

		const float modelRadius =
			std::max(
				glm::length(bounds.maximum - bounds.minimum) * 0.5f,
				0.001f);

		constexpr float verticalFieldOfView =
			glm::radians(45.0f);

		const auto initialExtent =
			m_Window.GetFramebufferExtent();

		const float initialAspectRatio =
			initialExtent.IsDrawable()
			? static_cast<float>(initialExtent.width) /
			static_cast<float>(initialExtent.height)
			: 1.0f;

		// Use the narrower field of view so portrait windows also fit.
		const float framingHalfAngle =
			std::atan(
				std::tan(verticalFieldOfView * 0.5f) *
				std::min(initialAspectRatio, 1.0f));

		const float cameraDistance =
			modelRadius / std::sin(framingHalfAngle) * 1.25f;

		const float nearPlane =
			std::max(modelRadius * 0.01f, 0.001f);

		const float farPlane =
			std::max(
				cameraDistance + modelRadius * 4.0f,
				10.0f);

		ForgeSim::Renderer::PerspectiveCamera camera{
			modelCenter +
				glm::vec3{
					0.0f,
					0.0f,
					cameraDistance
				},
			modelCenter,
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			verticalFieldOfView,
			nearPlane,
			farPlane
		};

		FreeCameraController cameraController{
			FreeCameraControllerSpecification{
				.movementSpeed =
					std::max(
						cameraDistance * 0.5f,
						0.01f),

				.mouseSensitivity = 0.002f,
				.maximumDeltaSeconds = 0.1f
			}
		};

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

		if (!m_LoadedScene && m_Scene.PointLights().empty())
		{
			static_cast<void>(m_Scene.AddPointLight());

			PointLight& pointLight =
				m_Scene.PointLights().back();

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

		if (!m_LoadedScene && m_Scene.SpotLights().empty())
		{
			static_cast<void>(m_Scene.AddSpotLight());

			SpotLight& spotLight =
				m_Scene.SpotLights().back();

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

		std::vector<LightPickTarget> lightPickTargets;

		while (!m_Window.ShouldClose())
		{
			m_Window.PollEvents();

			if (m_Window.ShouldClose())
			{
				break;
			}

			m_Renderer.ReloadChangedShaders();

			lightPickTargets.clear();

			if (m_DebugUI.ShowLights())
			{
				const LightDebugLayout layout =
					CalculateLightDebugLayout(
						m_Renderer.ObjectLocalBounds());

				lightPickTargets.push_back(
					LightPickTarget{
						.id = m_Scene.MainDirectionalLight().id,
						.bounds = {
							.center = m_Scene.MainDirectionalLight().position,
							.radius = layout.pickRadius
						}
					});

				for (const PointLight& light : m_Scene.PointLights())
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

				for (const SpotLight& light : m_Scene.SpotLights())
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

			const TransformEditorUIState editorUI =
				m_DebugUI.DrawTransformEditorControls(
					transformEditor.IsActive(),
					input.GetMouseButtonState(
						Platform::MouseButton::Right).held);

			runtimeDiagnostics.SetEnabled(
				m_DebugUI.RuntimeDiagnosticsEnabled());

			if (!runtimeDiagnostics.IsEnabled())
			{
				latestDiagnosticsSnapshot.reset();
			}

			const bool uiWantsMouse =
				m_DebugUI.WantsMouseCapture();

			const bool uiWantsKeyboard =
				m_DebugUI.WantsKeyboardCapture();

			const bool leftMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Left).held;

			const bool rightMouseDown =
				input.GetMouseButtonState(
					Platform::MouseButton::Right).held;

			if (const auto requestedCursorMode =
				cameraController.RequestedCursorMode(input))
			{
				const bool isReleaseRequest =
					*requestedCursorMode ==
					ForgeSim::Platform::CursorMode::Normal;

				if (isReleaseRequest ||
					(!uiWantsMouse &&
						!transformEditor.IsActive() &&
						!leftMouseDown))
				{
					m_Window.SetCursorMode(requestedCursorMode.value());
				}
			}

			const float aspectRatio =
				static_cast<float>(framebufferExtent.width) /
				static_cast<float>(framebufferExtent.height);

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

			if (rightMouseDown && 
				!uiWantsMouse &&
				!uiWantsKeyboard &&
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
				m_Scene);

			transformEditor.Update(
				SceneTransformEditorContext{
					.scene = m_Scene,
					.selection = selection,
					.view = view,
					.projection = projection,
					.operation = editorUI.operation,
					.space = editorUI.space,
					.showLights = m_DebugUI.ShowLights(),
					.navigationActive = rightMouseDown
				});

			if (editorUI.undoRequested)
			{
				static_cast<void>(transformEditor.Undo(m_Scene));
			}
			else if (editorUI.redoRequested)
			{
				static_cast<void>(transformEditor.Redo(m_Scene));
			}

			const bool pickingSuppressed =
				uiWantsMouse ||
				transformEditor.SuppressesPicking();



			selectionController.Update(
				input,
				m_Window.GetWindowExtent(),
				framebufferExtent,
				view,
				projection,
				m_Renderer.ObjectLocalBounds(),
				m_Scene.Objects(),
				lightPickTargets,
				selection,
				pickingSuppressed);

			const SandboxRenderSettings renderSettings{
				.framebufferWidth =
					static_cast<std::uint32_t>(
						framebufferExtent.width),

				.framebufferHeight =
					static_cast<std::uint32_t>(
						framebufferExtent.height),

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

				.selectedLightId = selection.SelectedLightId(),

				.selectionHighlight = {
					.enabled =
						m_DebugUI.SelectionHighlightEnabled(),

					.color =
						m_DebugUI.SelectionHighlightColor(),

					.strength =
						m_DebugUI.SelectionHighlightStrength()
					}
			};

			m_Renderer.Render(
				m_Scene.Objects(),
				selection.SelectedObjectId(),
				view,
				projection,
				camera.Position(),
				SceneLightingView{
					.directionalLight = m_Scene.MainDirectionalLight(),
					.pointLights = m_Scene.PointLights(),
					.spotLights = m_Scene.SpotLights()
				},
				renderSettings);

			m_DebugUI.Draw(
				SandboxDebugUIContext{
					.scene = m_Scene,
					.selection = selection,
					.diagnosticsSnapshot =
						latestDiagnosticsSnapshot 
							? &*latestDiagnosticsSnapshot 
							: nullptr,
					.camera = camera,
					.modelBounds = m_Renderer.ObjectLocalBounds(),
					.directionalLight = m_Scene.MainDirectionalLight(),
					.pointLights = m_Scene.PointLights(),
					.spotLights = m_Scene.SpotLights()
				}
			);

			m_DebugUI.EndFrame();

			m_Window.SwapBuffers();
		}
	}
}