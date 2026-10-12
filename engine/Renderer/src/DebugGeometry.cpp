#include <ForgeSim/Renderer/DebugGeometry.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>

#include <glm/vec4.hpp>

namespace ForgeSim::Renderer
{
	std::vector<DebugLineVertex>
		CreateReferenceGridLines(
			const ReferenceGridSpecification& specification)
	{
		if (specification.halfLineCount < 0)
		{
			throw std::invalid_argument(
				"Grid half-line count cannot be negative.");
		}

		if (specification.spacing <= 0.0f)
		{
			throw std::invalid_argument(
				"Grid spacing must be positive.");
		}

		const float extent =
			static_cast<float>(
				specification.halfLineCount) *
			specification.spacing;

		std::vector<DebugLineVertex> vertices;

		vertices.reserve(
			static_cast<std::size_t>(
				(specification.halfLineCount * 2 + 1) *
				4));

		for (int line = -specification.halfLineCount;
			line <= specification.halfLineCount;
			++line)
		{
			const float offset =
				static_cast<float>(line) *
				specification.spacing;

			const glm::vec3 xLineColor =
				line == 0
				? specification.xAxisColor
				: specification.lineColor;

			vertices.push_back(
				DebugLineVertex{
					.position = glm::vec3{
						-extent,
						specification.height,
						offset
					},
					.color = xLineColor
				});

			vertices.push_back(
				DebugLineVertex{
					.position = glm::vec3{
						extent,
						specification.height,
						offset
					},
					.color = xLineColor
				});

			const glm::vec3 zLineColor =
				line == 0
				? specification.zAxisColor
				: specification.lineColor;

			vertices.push_back(
				DebugLineVertex{
					.position = glm::vec3{
						offset,
						specification.height,
						-extent
					},
					.color = zLineColor
				});

			vertices.push_back(
				DebugLineVertex{
					.position = glm::vec3{
						offset,
						specification.height,
						extent
					},
					.color = zLineColor
				});
		}

		return vertices;
	}

	std::vector<DebugLineVertex>
		CreateWireframeBoxLines(
			const AxisAlignedBoundingBox& localBounds,
			const glm::mat4& modelMatrix,
			const glm::vec3& color)
	{
		if (localBounds.minimum.x > localBounds.maximum.x ||
			localBounds.minimum.y > localBounds.maximum.y ||
			localBounds.minimum.z > localBounds.maximum.z)
		{
			throw std::invalid_argument(
				"Wireframe-box minimum bounds cannot exceed "
				"maximum bounds.");
		}

		const std::array<glm::vec3, 8> localCorners{
			glm::vec3{
				localBounds.minimum.x,
				localBounds.minimum.y,
				localBounds.minimum.z
			},
			glm::vec3{
				localBounds.maximum.x,
				localBounds.minimum.y,
				localBounds.minimum.z
			},
			glm::vec3{
				localBounds.maximum.x,
				localBounds.maximum.y,
				localBounds.minimum.z
			},
			glm::vec3{
				localBounds.minimum.x,
				localBounds.maximum.y,
				localBounds.minimum.z
			},
			glm::vec3{
				localBounds.minimum.x,
				localBounds.minimum.y,
				localBounds.maximum.z
			},
			glm::vec3{
				localBounds.maximum.x,
				localBounds.minimum.y,
				localBounds.maximum.z
			},
			glm::vec3{
				localBounds.maximum.x,
				localBounds.maximum.y,
				localBounds.maximum.z
			},
			glm::vec3{
				localBounds.minimum.x,
				localBounds.maximum.y,
				localBounds.maximum.z
			}
		};

		std::array<glm::vec3, 8> worldCorners;

		for (std::size_t corner = 0;
			corner < localCorners.size();
			++corner)
		{
			worldCorners[corner] =
				glm::vec3{
					modelMatrix *
					glm::vec4{
						localCorners[corner],
						1.0f
					}
			};
		}

		constexpr std::array<
			std::array<std::size_t, 2>,
			12> edges{
			std::array<std::size_t, 2>{ 0, 1 },
			std::array<std::size_t, 2>{ 1, 2 },
			std::array<std::size_t, 2>{ 2, 3 },
			std::array<std::size_t, 2>{ 3, 0 },

			std::array<std::size_t, 2>{ 4, 5 },
			std::array<std::size_t, 2>{ 5, 6 },
			std::array<std::size_t, 2>{ 6, 7 },
			std::array<std::size_t, 2>{ 7, 4 },

			std::array<std::size_t, 2>{ 0, 4 },
			std::array<std::size_t, 2>{ 1, 5 },
			std::array<std::size_t, 2>{ 2, 6 },
			std::array<std::size_t, 2>{ 3, 7 }
		};

		std::vector<DebugLineVertex> vertices;
		vertices.reserve(edges.size() * 2);

		for (const auto& edge : edges)
		{
			vertices.push_back(
				DebugLineVertex{
					.position = worldCorners[edge[0]],
					.color = color
				});

			vertices.push_back(
				DebugLineVertex{
					.position = worldCorners[edge[1]],
					.color = color
				});
		}

		return vertices;
	}
}