#include "SandboxApplication.hpp"
#include "SandboxObject.hpp"
#include "FreeCameraController.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

#include <ForgeSim/Core/FixedStepScheduler.hpp>
#include <ForgeSim/Core/FrameStatistics.hpp>
#include <ForgeSim/Core/Log.hpp>
#include <ForgeSim/Core/Timer.hpp>
#include <ForgeSim/Platform/InputState.hpp>
#include <ForgeSim/Renderer/PerspectiveCamera.hpp>

#ifndef FORGESIM_SANDBOX_ASSET_DIRECTORY
#error FORGESIM_SANDBOX_ASSET_DIRECTORY must be defined by CMake
#endif

namespace ForgeSim::Sandbox
{
	namespace
	{
		struct Vertex
		{
			float position[3];
			float color[3];
		};

		[[nodiscard]] std::string LoadTextFile(
			const std::filesystem::path& path)
		{
			std::ifstream file{
				path,
				std::ios::in | std::ios::binary
			};

			if (!file)
			{
				throw std::runtime_error(
					"Failed to open text file: " +
					path.string());
			}

			std::ostringstream contents;
			contents << file.rdbuf();

			if (file.bad())
			{
				throw std::runtime_error(
					"Failed while reading text file: " +
					path.string());
			}

			return contents.str();
		}

		[[nodiscard]] ForgeSim::Renderer::OpenGL::ShaderProgram 
			CreateSandboxShaderProgram()
		{
			const std::filesystem::path shaderDirectory =
				std::filesystem::path{
				FORGESIM_SANDBOX_ASSET_DIRECTORY				
			} / "shaders";

			const std::filesystem::path vertexShaderPath =
				shaderDirectory / "Sandbox.vert";

			const std::filesystem::path fragmentShaderPath =
				shaderDirectory / "Sandbox.frag";

			const std::string vertexShaderSource =
				LoadTextFile(vertexShaderPath);

			const std::string fragmentShaderSource =
				LoadTextFile(fragmentShaderPath);

			return ForgeSim::Renderer::OpenGL::ShaderProgram{
				vertexShaderSource,
				fragmentShaderSource
			};
		}

		[[nodiscard]] std::vector<Vertex> CreateGridVertices()
		{
			constexpr int halfLineCount = 10;
			constexpr float spacing = 1.0f;
			constexpr float gridHeight = -0.75f;

			constexpr float gridColor = 0.35f;

			const float extent =
				static_cast<float>(halfLineCount) * spacing;

			std::vector<Vertex> gridVertices;

			gridVertices.reserve(
				static_cast<std::size_t>(
					(halfLineCount * 2 + 1) * 4));

			for (int line = -halfLineCount; line <= halfLineCount; ++line)
			{
				const float offset =
					static_cast<float>(line) * spacing;

				// Lines parallel to X. The center line represents the X axis.
				const float xAxisRed =
					line == 0 ? 0.9f : gridColor;

				const float xAxisGreen =
					line == 0 ? 0.2f : gridColor;

				const float xAxisBlue =
					line == 0 ? 0.2f : gridColor;

				gridVertices.push_back(
					Vertex{
						.position = {
							-extent,
							gridHeight,
							offset
						},
						.color = {
							xAxisRed,
							xAxisGreen,
							xAxisBlue
						}
						});

				gridVertices.push_back(
					Vertex{
						.position = {
							extent,
							gridHeight,
							offset
						},
						.color = {
							xAxisRed,
							xAxisGreen,
							xAxisBlue
						}
					});

				// Lines parallel to Z. The center line represents the Z axis.
				const float zAxisRed =
					line == 0 ? 0.2f : gridColor;

				const float zAxisGreen =
					line == 0 ? 0.4f : gridColor;

				const float zAxisBlue =
					line == 0 ? 0.9f : gridColor;

				gridVertices.push_back(
					Vertex{
						.position = {
							offset,
							gridHeight,
							-extent
						},
						.color = {
							zAxisRed,
							zAxisGreen,
							zAxisBlue
						}
					});

				gridVertices.push_back(
					Vertex{
						.position = {
							offset,
							gridHeight,
							extent
						},
						.color = {
							zAxisRed,
							zAxisGreen,
							zAxisBlue
						}
					});
			}
			return gridVertices;
		}

		constexpr std::array<Vertex, 8> vertices{
			Vertex{
				.position = { -0.5f, -0.5f, -0.5f },
				.color = { 1.0f, 0.0f, 0.0f }
			},
			Vertex{
				.position = { 0.5f, -0.5f, -0.5f },
				.color = { 0.0f, 1.0f, 0.0f }
			},
			Vertex{
				.position = { 0.5f, 0.5f, -0.5f },
				.color = { 0.0f, 0.0f, 1.0f }
			},
			Vertex{
				.position = { -0.5f, 0.5f, -0.5f },
				.color = { 1.0f, 1.0f, 0.0f }
			},
			Vertex{
				.position = { -0.5f, -0.5f, 0.5f },
				.color = { 1.0f, 0.0f, 1.0f }
			},
			Vertex{
				.position = { 0.5f, -0.5f, 0.5f },
				.color = { 0.0f, 1.0f, 1.0f }
			},
			Vertex{
				.position = { 0.5f, 0.5f, 0.5f },
				.color = { 1.0f, 1.0f, 1.0f }
			},
			Vertex{
				.position = { -0.5f, 0.5f, 0.5f },
				.color = { 0.3f, 0.3f, 0.3f }
			}
		};

		constexpr std::array<std::uint32_t, 36> indices{
			// Front
			4, 5, 6,
			6, 7, 4,

			// Back
			1, 0, 3,
			3, 2, 1,

			// Left
			0, 4, 7,
			7, 3, 0,

			// Right
			5, 1, 2,
			2, 6, 5,

			// Top
			3, 7, 6,
			6, 2, 3,

			// Bottom
			0, 1, 5,
			5, 4, 0
		};

		const std::vector<Vertex> gridVertices =
			CreateGridVertices();
	}

	SandboxApplication::SandboxApplication(
		const SandboxApplicationSpecification& spec)
		: m_Window(spec.windowSpec)
		, m_ShaderProgram(
			CreateSandboxShaderProgram())
		, m_VertexBuffer(
			vertices.data(),
			vertices.size() * sizeof(Vertex))
		, m_IndexBuffer(
			indices.data(),
			indices.size() * sizeof(std::uint32_t))
		, m_VertexArray()
		, m_GridVertexBuffer(
			gridVertices.data(),
			gridVertices.size() * sizeof(Vertex))
		, m_GridVertexArray()
	{
		constexpr std::uint32_t vertexBindingIndex = 0;

		m_VertexArray.SetVertexBuffer(
			m_VertexBuffer,
			vertexBindingIndex,
			sizeof(Vertex));

		m_VertexArray.SetIndexBuffer(m_IndexBuffer);

		m_VertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(Vertex, position));

		m_VertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(Vertex, color));

		m_GridVertexArray.SetVertexBuffer(
			m_GridVertexBuffer,
			vertexBindingIndex,
			sizeof(Vertex));

		m_GridVertexArray.SetFloatAttribute(
			0,
			vertexBindingIndex,
			3,
			offsetof(Vertex, position));

		m_GridVertexArray.SetFloatAttribute(
			1,
			vertexBindingIndex,
			3,
			offsetof(Vertex, color));
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

		glClearColor(0.05f, 0.15f, 0.30f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		ForgeSim::Renderer::PerspectiveCamera camera{
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::radians(45.0f),
			0.1f,
			100.0f
		};

		FreeCameraController cameraController;

		std::array<SandboxObject, 3> objects{
			SandboxObject{
				.id = 1,
				.transform = Transform{
					.position = glm::vec3{ -1.25f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(25.0f),
						glm::radians(35.0f),
						0.0f
					},
				.scale = glm::vec3{ 0.75f }
				}
			},
			SandboxObject{
				.id = 2,
				.transform = Transform{
					.position = glm::vec3{ 0.0f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(15.0f),
						glm::radians(-20.0f),
						0.0f
					},
					.scale = glm::vec3{ 1.0f }
				}
			},
			SandboxObject{
				.id = 3,
				.transform = Transform{
					.position = glm::vec3{ 1.25f, 0.0f, 0.0f },
					.rotationRadians = glm::vec3{
						glm::radians(-15.0f),
						glm::radians(30.0f),
						0.0f
					},
					.scale = glm::vec3{ 0.6f }
				}
			}
		};

		ForgeSim::Core::Timer timer;
		ForgeSim::Core::FrameStatistics frameStats;

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

		std::uint64_t fixedUpdateCount = 0;

		while (!m_Window.ShouldClose())
		{
			m_Window.PollEvents();

			if (m_Window.ShouldClose())
			{
				break;
			}

			const ForgeSim::Platform::InputState& input =
				m_Window.GetInputState();

			const auto cameraLookState =
				input.GetMouseButtonState(
					ForgeSim::Platform::MouseButton::Right);

			if (cameraLookState.pressed)
			{
				m_Window.SetCursorMode(
					ForgeSim::Platform::CursorMode::Captured);
			}

			if (cameraLookState.released)
			{
				m_Window.SetCursorMode(
					ForgeSim::Platform::CursorMode::Normal);
			}

			const auto framebufferExtent =
				m_Window.GetFramebufferExtent();

			if (!framebufferExtent.IsDrawable())
			{
				m_Window.WaitEvents();
				static_cast<void>(timer.Restart());
				continue;
			}

			const float aspectRatio =
				static_cast<float>(framebufferExtent.width) /
				static_cast<float>(framebufferExtent.height);

			const glm::mat4 projection =
				camera.ProjectionMatrix(aspectRatio);

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
				++fixedUpdateCount;
			}

			if (fixedStepResult.droppedTime >
				FixedStepScheduler::Duration::zero())
			{
				const double droppedMilliseconds =
					std::chrono::duration<double, std::milli>{
						fixedStepResult.droppedTime
				}.count();

				ForgeSim::Core::LogInfo(
					"Dropped simulation time: " +
					std::to_string(droppedMilliseconds) +
					"ms");
			}

			if (const auto snapshot =
				frameStats.AddFrame(frameDelta))
			{
				ForgeSim::Core::LogInfo(
					"FPS: " +
					std::to_string(snapshot->framesPerSecond) +
					", Average Frame Time: " +
					std::to_string(
						snapshot->averageFrameTimeMilliseconds) +
					" ms | Fixed updates: " +
					std::to_string(fixedUpdateCount));

				fixedUpdateCount = 0;
			}

			glClear(
				GL_COLOR_BUFFER_BIT |
				GL_DEPTH_BUFFER_BIT);

			cameraController.Update(
				camera,
				input,
				deltaSeconds);

			m_ShaderProgram.SetMatrix4x4(
				"uView",
				camera.ViewMatrix());

			m_ShaderProgram.SetMatrix4x4(
				"uProjection",
				projection);

			m_ShaderProgram.Bind();

			m_ShaderProgram.SetMatrix4x4(
				"uModel",
				glm::mat4{ 1.0f });

			m_GridVertexArray.Bind();

			glDrawArrays(
				GL_LINES,
				0,
				static_cast<GLsizei>(gridVertices.size()));

			m_VertexArray.Bind();

			for (const SandboxObject& object : objects)
			{
				m_ShaderProgram.SetMatrix4x4(
					"uModel",
					object.transform.ModelMatrix());

				glDrawElements(
					GL_TRIANGLES,
					static_cast<GLsizei>(indices.size()),
					GL_UNSIGNED_INT,
					nullptr);
			}

			m_Window.SwapBuffers();
		}
	}
}