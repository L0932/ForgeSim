#include <ForgeSim/Renderer/DebugGeometry.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

namespace ForgeSim::Renderer
{
	void AppendMarkerLines(
		std::vector<DebugLineVertex>& vertices,
		const glm::vec3& center,
		float markerSize,
		const glm::vec3& color)
	{
		if (!std::isfinite(markerSize) ||
			markerSize < 0.0f)
		{
			throw std::invalid_argument(
				"Marker size must be finite and nonnegative.");
		}

		if (markerSize == 0.0f)
		{
			return;
		}

		const std::array<glm::vec3, 3> axes{
			glm::vec3{ markerSize, 0.0f, 0.0f },
			glm::vec3{ 0.0f, markerSize, 0.0f },
			glm::vec3{ 0.0f, 0.0f, markerSize }
		};

		for (const glm::vec3& axis : axes)
		{
			const glm::vec3 start = center - axis;
			const glm::vec3 end = center + axis;

			for (int component = 0; component < 3; ++component)
			{
				if (!std::isfinite(start[component]) ||
					!std::isfinite(end[component]))
				{
					throw std::invalid_argument(
						"Marker coordinates exceed "
						"the supported numeric range.");
				}
			}

			vertices.push_back(
				DebugLineVertex{
					.position = start,
					.color = color
				});

			vertices.push_back(
				DebugLineVertex{
					.position = end,
					.color = color
				});
		}
	}

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

	std::vector<DebugLineVertex>
		CreateDirectionArrowLines(
			const DirectionArrowSpecification& specification)
	{
		const auto isFinite =
			[](const glm::vec3& value)
			{
				return
					std::isfinite(value.x) &&
					std::isfinite(value.y) &&
					std::isfinite(value.z);
			};

		if (!isFinite(specification.origin) ||
			!isFinite(specification.direction) ||
			!isFinite(specification.color) ||
			!std::isfinite(specification.length) ||
			specification.length <= 0.0f)
		{
			throw std::invalid_argument(
				"Direction arrow requires finite values "
				"and a positive length.");
		}

		const float directionLengthSquared =
			glm::dot(
				specification.direction,
				specification.direction);

		if (!std::isfinite(directionLengthSquared) ||
			directionLengthSquared <= 1e-12f)
		{
			throw std::invalid_argument(
				"Direction arrow requires a nonzero "
				"direction with a finite squared length.");
		}

		const glm::vec3 forward =
			specification.direction /
			std::sqrt(directionLengthSquared);

		const glm::vec3 referenceAxis =
			std::abs(forward.z) < 0.9f
			? glm::vec3{ 0.0f, 0.0f, 1.0f }
		: glm::vec3{ 0.0f, 1.0f, 0.0f };

		const glm::vec3 right =
			glm::normalize(
				glm::cross(forward, referenceAxis));

		const glm::vec3 up =
			glm::cross(right, forward);

		const glm::vec3 tip =
			specification.origin +
			forward * specification.length;

		const glm::vec3 headCenter =
			specification.origin +
			forward * (specification.length * 0.75f);

		const float headRadius =
			specification.length * 0.10f;

		const std::array<glm::vec3, 4> headPoints{
			headCenter + right * headRadius,
			headCenter - right * headRadius,
			headCenter + up * headRadius,
			headCenter - up * headRadius
		};

		std::vector<DebugLineVertex> vertices;
		vertices.reserve(10);

		const auto appendLine =
			[&](const glm::vec3& start, const glm::vec3& end)
			{
				if (!isFinite(start) || !isFinite(end))
				{
					throw std::invalid_argument(
						"Direction arrow coordinates exceed "
						"the supported numeric range.");
				}

				vertices.push_back(
					DebugLineVertex{
						.position = start,
						.color = specification.color
					});

				vertices.push_back(
					DebugLineVertex{
						.position = end,
						.color = specification.color
					});
			};

		appendLine(specification.origin, tip);

		if (specification.showArrowheads)
		{
			for (const glm::vec3& headPoint : headPoints)
			{
				appendLine(tip, headPoint);
			}
		}

		return vertices;
	}

	std::vector<DebugLineVertex>
		CreateDirectionalLightLines(
			const DirectionArrowSpecification& specification)
	{
		// Reuse the arrow generator's input validation.
		std::vector<DebugLineVertex> vertices =
			CreateDirectionArrowLines(specification);

		constexpr int rayCount = 8;

		vertices.reserve(
			10 + rayCount * 12);

		const glm::vec3 forward =
			glm::normalize(specification.direction);

		const glm::vec3 referenceAxis =
			std::abs(forward.z) < 0.9f
			? glm::vec3{ 0.0f, 0.0f, 1.0f }
		: glm::vec3{ 0.0f, 1.0f, 0.0f };

		const glm::vec3 right =
			glm::normalize(
				glm::cross(forward, referenceAxis));

		const glm::vec3 up =
			glm::cross(right, forward);

		const float radius =
			specification.length * 0.25f;

		const auto ringPoint =
			[&](int index)
			{
				const float angle =
					static_cast<float>(index % rayCount) /
					static_cast<float>(rayCount) *
					2.0f * std::numbers::pi_v<float>;

				return specification.origin +
					radius * (
						right * std::cos(angle) +
						up * std::sin(angle));
			};

		for (int index = 0; index < rayCount; ++index)
		{
			const glm::vec3 start = ringPoint(index);
			const glm::vec3 end = ringPoint(index + 1);

			const auto isFinite =
				[](const glm::vec3& value)
				{
					return
						std::isfinite(value.x) &&
						std::isfinite(value.y) &&
						std::isfinite(value.z);
				};

			if (!isFinite(start) || !isFinite(end))
			{
				throw std::invalid_argument(
					"Directional-light symbol coordinates "
					"exceed the supported numeric range.");
			}

			vertices.push_back(
				DebugLineVertex{
					.position = start,
					.color = specification.color
				});

			vertices.push_back(
				DebugLineVertex{
					.position = end,
					.color = specification.color
				});

			const auto rayVertices =
				CreateDirectionArrowLines(
					DirectionArrowSpecification{
						.origin = start,
						.direction = specification.direction,
						.length = specification.length,
						.color = specification.color,
						.showArrowheads = specification.showArrowheads
					});

			vertices.insert(
				vertices.end(),
				rayVertices.begin(),
				rayVertices.end());
		}

		AppendMarkerLines(
			vertices,
			specification.origin,
			specification.markerSize,
			specification.color);

		return vertices;
	}

	std::vector<DebugLineVertex>
		CreateWireframeSphereLines(
			const WireframeSphereSpecification& specification)
	{
		const auto isFinite =
			[](const glm::vec3& value)
			{
				return
					std::isfinite(value.x) &&
					std::isfinite(value.y) &&
					std::isfinite(value.z);
			};

		if (!isFinite(specification.center) ||
			!isFinite(specification.color) ||
			!std::isfinite(specification.radius) ||
			specification.radius <= 0.0f ||
			!std::isfinite(specification.markerSize) ||
			specification.markerSize < 0.0f ||
			specification.segmentCount < 3 ||
			specification.segmentCount > 4096)
		{
			throw std::invalid_argument(
				"Wireframe sphere requires finite values, "
				"a positive radius, a nonnegative marker size, "
				"and between 3 and 4096 segments.");
		}

		std::vector<DebugLineVertex> vertices;

		vertices.reserve(
			static_cast<std::size_t>(
				specification.segmentCount) * 6 +
			6);

		const auto appendLine =
			[&](const glm::vec3& start, const glm::vec3& end)
			{
				if (!isFinite(start) || !isFinite(end))
				{
					throw std::invalid_argument(
						"Wireframe-sphere coordinates exceed "
						"the supported numeric range.");
				}

				vertices.push_back(
					DebugLineVertex{
						.position = start,
						.color = specification.color
					});

				vertices.push_back(
					DebugLineVertex{
						.position = end,
						.color = specification.color
					});
			};

		// Three perpendicular great circles: XY, XZ, and YZ.
		for (int plane = 0; plane < 3; ++plane)
		{
			const auto circlePoint =
				[&](int index)
				{
					const float angle =
						static_cast<float>(
							index % specification.segmentCount) /
						static_cast<float>(
							specification.segmentCount) *
						2.0f * std::numbers::pi_v<float>;

					const float first =
						specification.radius * std::cos(angle);

					const float second =
						specification.radius * std::sin(angle);

					glm::vec3 offset{ 0.0f };

					if (plane == 0)
					{
						offset = { first, second, 0.0f };
					}
					else if (plane == 1)
					{
						offset = { first, 0.0f, second };
					}
					else
					{
						offset = { 0.0f, first, second };
					}

					return specification.center + offset;
				};

			for (int segment = 0;
				segment < specification.segmentCount;
				++segment)
			{
				appendLine(
					circlePoint(segment),
					circlePoint(segment + 1));
			}
		}

		AppendMarkerLines(
			vertices,
			specification.center,
			specification.markerSize,
			specification.color);

		return vertices;
	}

	std::vector<DebugLineVertex>
		CreateWireframeSpotLightLines(
			const WireframeSpotLightSpecification& specification)
	{
		const auto isFinite =
			[](const glm::vec3& value)
			{
				return
					std::isfinite(value.x) &&
					std::isfinite(value.y) &&
					std::isfinite(value.z);
			};

		if (!isFinite(specification.position) ||
			!isFinite(specification.emissionDirection) ||
			!isFinite(specification.color) ||
			!std::isfinite(specification.range) ||
			specification.range <= 0.0f ||
			!std::isfinite(specification.markerSize) ||
			specification.markerSize < 0.0f ||
			!std::isfinite(specification.innerConeAngleRadians) ||
			!std::isfinite(specification.outerConeAngleRadians) ||
			specification.innerConeAngleRadians < 0.0f ||
			specification.innerConeAngleRadians >=
			specification.outerConeAngleRadians ||
			specification.outerConeAngleRadians >=
			std::numbers::pi_v<float> *0.5f ||
			specification.segmentCount < 3 ||
			specification.segmentCount > 4096)
		{
			throw std::invalid_argument(
				"Spot-light geometry requires finite values, "
				"positive range, nonnegative marker size, "
				"3 to 4096 segments, and cone half-angles satisfying "
				"0 <= inner < outer < pi / 2.");
		}

		const float directionLengthSquared =
			glm::dot(
				specification.emissionDirection,
				specification.emissionDirection);

		if (!std::isfinite(directionLengthSquared) ||
			directionLengthSquared <= 1e-12f)
		{
			throw std::invalid_argument(
				"Spot-light geometry requires a nonzero direction "
				"with a finite squared length.");
		}

		const glm::vec3 forward =
			specification.emissionDirection /
			std::sqrt(directionLengthSquared);

		const glm::vec3 referenceAxis =
			std::abs(forward.z) < 0.9f
			? glm::vec3{ 0.0f, 0.0f, 1.0f }
		: glm::vec3{ 0.0f, 1.0f, 0.0f };

		const glm::vec3 right =
			glm::normalize(
				glm::cross(forward, referenceAxis));

		const glm::vec3 up =
			glm::cross(right, forward);

		std::vector<DebugLineVertex> vertices;

		vertices.reserve(
			static_cast<std::size_t>(
				specification.segmentCount) * 4 + 24);

		const auto appendLine =
			[&](const glm::vec3& start, const glm::vec3& end)
			{
				if (!isFinite(start) || !isFinite(end))
				{
					throw std::invalid_argument(
						"Spot-light geometry coordinates exceed "
						"the supported numeric range.");
				}

				vertices.push_back(
					DebugLineVertex{
						.position = start,
						.color = specification.color
					});

				vertices.push_back(
					DebugLineVertex{
						.position = end,
						.color = specification.color
					});
			};

		appendLine(
			specification.position,
			specification.position +
			forward * specification.range);

		const auto appendCone =
			[&](float halfAngle)
			{
				// A zero-width inner cone coincides with the center line.
				if (halfAngle == 0.0f)
				{
					return;
				}

				const float axialDistance =
					specification.range * std::cos(halfAngle);

				const float radius =
					specification.range * std::sin(halfAngle);

				const glm::vec3 ringCenter =
					specification.position +
					forward * axialDistance;

				const auto ringPoint =
					[&](float angle)
					{
						return ringCenter +
							radius * (
								right * std::cos(angle) +
								up * std::sin(angle));
					};

				for (int segment = 0;
					segment < specification.segmentCount;
					++segment)
				{
					const float startAngle =
						static_cast<float>(segment) /
						static_cast<float>(
							specification.segmentCount) *
						2.0f * std::numbers::pi_v<float>;

					const float endAngle =
						static_cast<float>(
							(segment + 1) %
							specification.segmentCount) /
						static_cast<float>(
							specification.segmentCount) *
						2.0f * std::numbers::pi_v<float>;

					appendLine(
						ringPoint(startAngle),
						ringPoint(endAngle));
				}

				// Four boundary rays make the cone readable in 3D.
				for (int side = 0; side < 4; ++side)
				{
					const float angle =
						static_cast<float>(side) *
						std::numbers::pi_v<float> *0.5f;

					appendLine(
						specification.position,
						ringPoint(angle));
				}
			};

		appendCone(specification.outerConeAngleRadians);
		appendCone(specification.innerConeAngleRadians);

		AppendMarkerLines(
			vertices,
			specification.position,
			specification.markerSize,
			specification.color);

		return vertices;
	}
}