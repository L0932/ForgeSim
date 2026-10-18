#include <ForgeSim/Renderer/DebugGeometry.hpp>

#include <array>
#include <cstddef>
#include <limits>
#include <numbers>
#include <stdexcept>

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace
{
	[[nodiscard]] bool NearlyEqual(
		float left,
		float right,
		float tolerance = 1e-5f) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}

	[[nodiscard]] bool NearlyEqual(
		const glm::vec3& left,
		const glm::vec3& right,
		float tolerance = 1e-5f) noexcept
	{
		return
			NearlyEqual(left.x, right.x, tolerance) &&
			NearlyEqual(left.y, right.y, tolerance) &&
			NearlyEqual(left.z, right.z, tolerance);
	}
}

int main()
{
	using ForgeSim::Renderer::AxisAlignedBoundingBox;
	using ForgeSim::Renderer::CreateReferenceGridLines;
	using ForgeSim::Renderer::CreateWireframeBoxLines;
	using ForgeSim::Renderer::ReferenceGridSpecification;
	using ForgeSim::Renderer::CreateDirectionArrowLines;
	using ForgeSim::Renderer::DirectionArrowSpecification;

	// A three-line grid in each direction produces
	// six lines and twelve vertices.
	{
		const ReferenceGridSpecification specification{
			.halfLineCount = 1,
			.spacing = 2.0f,
			.height = -1.0f
		};

		const auto vertices =
			CreateReferenceGridLines(specification);

		if (vertices.size() != 12)
		{
			std::cerr
				<< "Expected twelve reference-grid vertices\n";

			return EXIT_FAILURE;
		}

		if (!NearlyEqual(
			vertices.front().position,
			glm::vec3{ -2.0f, -1.0f, -2.0f }))
		{
			std::cerr
				<< "Reference grid had an unexpected extent\n";

			return EXIT_FAILURE;
		}

		// Vertices 4 and 5 form the central X-axis line.
		if (!NearlyEqual(
			vertices[4].color,
			specification.xAxisColor))
		{
			std::cerr
				<< "Central X-axis color was not applied\n";

			return EXIT_FAILURE;
		}

		// Vertices 6 and 7 form the central Z-axis line.
		if (!NearlyEqual(
			vertices[6].color,
			specification.zAxisColor))
		{
			std::cerr
				<< "Central Z-axis color was not applied\n";

			return EXIT_FAILURE;
		}
	}

	// A wireframe box has twelve edges and therefore
	// twenty-four line vertices.
	{
		const AxisAlignedBoundingBox bounds{
			.minimum = glm::vec3{ -0.5f },
			.maximum = glm::vec3{ 0.5f }
		};

		const glm::mat4 model =
			glm::translate(
				glm::mat4{ 1.0f },
				glm::vec3{ 1.0f, 2.0f, 3.0f });

		const glm::vec3 color{
			0.25f,
			0.5f,
			0.75f
		};

		const auto vertices =
			CreateWireframeBoxLines(
				bounds,
				model,
				color);

		if (vertices.size() != 24)
		{
			std::cerr
				<< "Expected twenty-four wireframe-box vertices\n";

			return EXIT_FAILURE;
		}

		glm::vec3 generatedMinimum =
			vertices.front().position;

		glm::vec3 generatedMaximum =
			vertices.front().position;

		for (const auto& vertex : vertices)
		{
			generatedMinimum =
				glm::min(
					generatedMinimum,
					vertex.position);

			generatedMaximum =
				glm::max(
					generatedMaximum,
					vertex.position);

			if (!NearlyEqual(vertex.color, color))
			{
				std::cerr
					<< "Wireframe-box color was not preserved\n";

				return EXIT_FAILURE;
			}
		}

		if (!NearlyEqual(
			generatedMinimum,
			glm::vec3{ 0.5f, 1.5f, 2.5f }) ||
			!NearlyEqual(
				generatedMaximum,
				glm::vec3{ 1.5f, 2.5f, 3.5f }))
		{
			std::cerr
				<< "Wireframe-box transform was not applied correctly\n";

			return EXIT_FAILURE;
		}
	}

	// Arrows should preserve origin, length, direction, and color.
	// Include directions exercising both reference-axis branches.
	{
		const std::array<glm::vec3, 4> directions{
			glm::vec3{ 0.0f, -2.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 3.0f },
			glm::vec3{ 0.0f, 0.0f, -4.0f },
			glm::vec3{ 1.0f, 2.0f, 3.0f }
		};

		for (const glm::vec3& direction : directions)
		{
			const DirectionArrowSpecification specification{
				.origin = { 1.0f, 2.0f, 3.0f },
				.direction = direction,
				.length = 2.0f,
				.color = { 0.25f, 0.5f, 0.75f }
			};

			const auto vertices =
				CreateDirectionArrowLines(specification);

			if (vertices.size() != 10)
			{
				std::cerr << "Expected ten direction-arrow vertices\n";
				return EXIT_FAILURE;
			}

			const glm::vec3 forward =
				glm::normalize(direction);

			const glm::vec3 expectedTip =
				specification.origin +
				forward * specification.length;

			if (!NearlyEqual(
				vertices[0].position,
				specification.origin) ||
				!NearlyEqual(
					vertices[1].position,
					expectedTip))
			{
				std::cerr << "Arrow shaft had incorrect endpoints\n";
				return EXIT_FAILURE;
			}

			for (const auto& vertex : vertices)
			{
				if (!std::isfinite(vertex.position.x) ||
					!std::isfinite(vertex.position.y) ||
					!std::isfinite(vertex.position.z) ||
					!NearlyEqual(vertex.color, specification.color))
				{
					std::cerr << "Arrow contained invalid position or color\n";
					return EXIT_FAILURE;
				}
			}

			const glm::vec3 headCenter =
				specification.origin +
				forward * (specification.length * 0.75f);

			for (std::size_t index = 2;
				index < vertices.size();
				index += 2)
			{
				const glm::vec3 radialOffset =
					vertices[index + 1].position - headCenter;

				if (!NearlyEqual(vertices[index].position, expectedTip) ||
					!NearlyEqual(glm::dot(radialOffset, forward), 0.0f) ||
					!NearlyEqual(
						glm::length(radialOffset),
						specification.length * 0.10f))
				{
					std::cerr << "Arrowhead geometry was incorrect\n";
					return EXIT_FAILURE;
				}
			}
		}
	}

	// Invalid arrow specifications should be rejected explicitly.
	{
		std::array<DirectionArrowSpecification, 6> invalidSpecifications{};

		invalidSpecifications[0].direction = glm::vec3{ 0.0f };
		invalidSpecifications[1].length = 0.0f;
		invalidSpecifications[2].length = -1.0f;

		invalidSpecifications[3].origin.x =
			std::numeric_limits<float>::quiet_NaN();

		invalidSpecifications[4].direction.y =
			std::numeric_limits<float>::infinity();

		invalidSpecifications[5].length =
			std::numeric_limits<float>::infinity();

		for (const auto& specification : invalidSpecifications)
		{
			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					CreateDirectionArrowLines(specification));
			}
			catch (const std::invalid_argument&)
			{
				exceptionWasThrown = true;
			}

			if (!exceptionWasThrown)
			{
				std::cerr << "Expected invalid arrow input to be rejected\n";
				return EXIT_FAILURE;
			}
		}
	}

	// Directional-light symbols should contain a ring and nine
	// parallel rays, with optional arrowheads.
	{
		using ForgeSim::Renderer::CreateDirectionalLightLines;

		for (const bool showArrowheads : { false, true })
		{
			const DirectionArrowSpecification specification{
				.origin = { 1.0f, 2.0f, 3.0f },
				.direction = { 0.0f, -2.0f, 0.0f },
				.length = 2.0f,
				.color = { 1.0f, 0.85f, 0.15f },
				.showArrowheads = showArrowheads
			};

			const auto vertices =
				CreateDirectionalLightLines(specification);

			const std::size_t rayVertexCount =
				showArrowheads ? 10 : 2;

			// Nine rays plus eight ring segments.
			const std::size_t expectedVertexCount =
				9 * rayVertexCount + 16;

			if (vertices.size() != expectedVertexCount)
			{
				std::cerr
					<< "Unexpected directional-light symbol vertex count\n";

				return EXIT_FAILURE;
			}

			const glm::vec3 expectedRayOffset{
				0.0f,
				-specification.length,
				0.0f
			};

			const auto checkRay =
				[&](std::size_t firstVertex)
				{
					return NearlyEqual(
						vertices[firstVertex + 1].position -
						vertices[firstVertex].position,
						expectedRayOffset);
				};

			if (!checkRay(0))
			{
				std::cerr << "Central light ray had incorrect direction\n";
				return EXIT_FAILURE;
			}

			for (std::size_t ray = 0; ray < 8; ++ray)
			{
				// Each surrounding group contains a ring segment
				// followed by that ring point's ray.
				const std::size_t ringStart =
					rayVertexCount + ray * (2 + rayVertexCount);

				const std::size_t rayStart =
					ringStart + 2;

				if (!checkRay(rayStart) ||
					!NearlyEqual(
						vertices[ringStart].position,
						vertices[rayStart].position))
				{
					std::cerr
						<< "Surrounding light ray was not parallel "
						<< "or did not start on the ring\n";

					return EXIT_FAILURE;
				}

				for (std::size_t endpoint = 0; endpoint < 2; ++endpoint)
				{
					const glm::vec3 radialOffset =
						vertices[ringStart + endpoint].position -
						specification.origin;

					if (!NearlyEqual(
						glm::length(radialOffset),
						specification.length * 0.25f) ||
						!NearlyEqual(radialOffset.y, 0.0f))
					{
						std::cerr
							<< "Directional-light ring had incorrect geometry\n";

						return EXIT_FAILURE;
					}
				}
			}

			for (const auto& vertex : vertices)
			{
				if (!std::isfinite(vertex.position.x) ||
					!std::isfinite(vertex.position.y) ||
					!std::isfinite(vertex.position.z) ||
					!NearlyEqual(vertex.color, specification.color))
				{
					std::cerr
						<< "Directional-light symbol contained invalid vertices\n";

					return EXIT_FAILURE;
				}
			}
		}
	}

	// Range-sphere vertices should lie at the requested radius.
	// The optional center marker should use its own world-space size.
	{
		using ForgeSim::Renderer::CreateWireframeSphereLines;
		using ForgeSim::Renderer::WireframeSphereSpecification;

		for (const bool showMarker : { false, true })
		{
			const WireframeSphereSpecification specification{
				.center = { 1.0f, 2.0f, 3.0f },
				.radius = 2.0f,
				.segmentCount = 16,
				.markerSize = showMarker ? 0.1f : 0.0f,
				.color = { 0.2f, 0.8f, 1.0f }
			};

			const auto vertices =
				CreateWireframeSphereLines(specification);

			const std::size_t circleVertexCount =
				static_cast<std::size_t>(
					specification.segmentCount) * 6;

			const std::size_t expectedVertexCount =
				circleVertexCount + (showMarker ? 6 : 0);

			if (vertices.size() != expectedVertexCount)
			{
				std::cerr << "Unexpected sphere vertex count\n";
				return EXIT_FAILURE;
			}

			for (const auto& vertex : vertices)
			{
				if (!std::isfinite(vertex.position.x) ||
					!std::isfinite(vertex.position.y) ||
					!std::isfinite(vertex.position.z) ||
					!NearlyEqual(vertex.color, specification.color))
				{
					std::cerr << "Sphere contained invalid position or color\n";
					return EXIT_FAILURE;
				}
			}

			const std::size_t verticesPerCircle =
				static_cast<std::size_t>(
					specification.segmentCount) * 2;

			for (std::size_t plane = 0; plane < 3; ++plane)
			{
				const std::size_t circleStart =
					plane * verticesPerCircle;

				for (std::size_t index = 0;
					index < verticesPerCircle;
					++index)
				{
					const glm::vec3 offset =
						vertices[circleStart + index].position -
						specification.center;

					const float perpendicularComponent =
						plane == 0 ? offset.z :
						plane == 1 ? offset.y :
						offset.x;

					if (!NearlyEqual(
						glm::length(offset),
						specification.radius) ||
						!NearlyEqual(perpendicularComponent, 0.0f))
					{
						std::cerr << "Sphere circle had incorrect geometry\n";
						return EXIT_FAILURE;
					}
				}

				// Consecutive segments must connect, including closure.
				for (std::size_t segment = 0;
					segment <
					static_cast<std::size_t>(
						specification.segmentCount);
					++segment)
				{
					const std::size_t nextSegment =
						(segment + 1) %
						static_cast<std::size_t>(
							specification.segmentCount);

					if (!NearlyEqual(
						vertices[circleStart + segment * 2 + 1].position,
						vertices[circleStart + nextSegment * 2].position))
					{
						std::cerr << "Sphere circle was not connected\n";
						return EXIT_FAILURE;
					}
				}
			}

			if (showMarker)
			{
				const std::array<glm::vec3, 3> axes{
					glm::vec3{ specification.markerSize, 0.0f, 0.0f },
					glm::vec3{ 0.0f, specification.markerSize, 0.0f },
					glm::vec3{ 0.0f, 0.0f, specification.markerSize }
				};

				for (std::size_t axis = 0; axis < axes.size(); ++axis)
				{
					const std::size_t start =
						circleVertexCount + axis * 2;

					if (!NearlyEqual(
						vertices[start].position,
						specification.center - axes[axis]) ||
						!NearlyEqual(
							vertices[start + 1].position,
							specification.center + axes[axis]))
					{
						std::cerr << "Sphere center marker was incorrect\n";
						return EXIT_FAILURE;
					}
				}
			}
		}
	}

	// Invalid sphere specifications should be rejected.
	{
		using ForgeSim::Renderer::CreateWireframeSphereLines;
		using ForgeSim::Renderer::WireframeSphereSpecification;

		std::array<WireframeSphereSpecification, 8>
			invalidSpecifications{};

		invalidSpecifications[0].radius = 0.0f;
		invalidSpecifications[1].radius = -1.0f;
		invalidSpecifications[2].segmentCount = 2;
		invalidSpecifications[3].segmentCount = 4097;
		invalidSpecifications[4].markerSize = -0.1f;

		invalidSpecifications[5].center.x =
			std::numeric_limits<float>::quiet_NaN();

		invalidSpecifications[6].radius =
			std::numeric_limits<float>::infinity();

		invalidSpecifications[7].markerSize =
			std::numeric_limits<float>::quiet_NaN();

		for (const auto& specification : invalidSpecifications)
		{
			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					CreateWireframeSphereLines(specification));
			}
			catch (const std::invalid_argument&)
			{
				exceptionWasThrown = true;
			}

			if (!exceptionWasThrown)
			{
				std::cerr << "Expected invalid sphere input to be rejected\n";
				return EXIT_FAILURE;
			}
		}
	}

	// Spot-light geometry should follow its position and direction.
// Cone endpoints should respect range and cone half-angles.
	{
		using ForgeSim::Renderer::CreateWireframeSpotLightLines;
		using ForgeSim::Renderer::WireframeSpotLightSpecification;

		const std::array<glm::vec3, 2> directions{
			glm::vec3{ 0.0f, -2.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 3.0f }
		};

		for (const glm::vec3& direction : directions)
		{
			for (const bool zeroInnerCone : { false, true })
			{
				const WireframeSpotLightSpecification specification{
					.position = { 1.0f, 2.0f, 3.0f },
					.emissionDirection = direction,
					.range = 2.0f,
					.innerConeAngleRadians =
						zeroInnerCone ? 0.0f : 0.25f,
					.outerConeAngleRadians = 0.5f,
					.segmentCount = 16,
					.markerSize = 0.1f,
					.color = { 1.0f, 0.5f, 0.2f }
				};

				const auto vertices =
					CreateWireframeSpotLightLines(specification);

				const std::size_t verticesPerCone =
					static_cast<std::size_t>(
						specification.segmentCount) * 2 + 8;

				const std::size_t coneCount =
					zeroInnerCone ? 1 : 2;

				if (vertices.size() !=
					2 + coneCount * verticesPerCone + 6)
				{
					std::cerr << "Unexpected spot-light vertex count\n";
					return EXIT_FAILURE;
				}

				const glm::vec3 forward =
					glm::normalize(direction);

				if (!NearlyEqual(
					vertices[0].position,
					specification.position) ||
					!NearlyEqual(
						vertices[1].position,
						specification.position +
						forward * specification.range))
				{
					std::cerr << "Spot-light center line was incorrect\n";
					return EXIT_FAILURE;
				}

				for (const auto& vertex : vertices)
				{
					if (!std::isfinite(vertex.position.x) ||
						!std::isfinite(vertex.position.y) ||
						!std::isfinite(vertex.position.z) ||
						!NearlyEqual(vertex.color, specification.color))
					{
						std::cerr << "Spot-light vertices were invalid\n";
						return EXIT_FAILURE;
					}
				}

				for (std::size_t cone = 0; cone < coneCount; ++cone)
				{
					const float halfAngle =
						cone == 0
						? specification.outerConeAngleRadians
						: specification.innerConeAngleRadians;

					const std::size_t coneStart =
						2 + cone * verticesPerCone;

					const std::size_t ringVertexCount =
						static_cast<std::size_t>(
							specification.segmentCount) * 2;

					const auto boundaryIsCorrect =
						[&](const glm::vec3& endpoint)
						{
							const glm::vec3 offset =
								endpoint - specification.position;

							const float axialDistance =
								glm::dot(offset, forward);

							const glm::vec3 radialOffset =
								offset - forward * axialDistance;

							return
								NearlyEqual(
									glm::length(offset),
									specification.range) &&
								NearlyEqual(
									axialDistance,
									specification.range *
									std::cos(halfAngle)) &&
								NearlyEqual(
									glm::length(radialOffset),
									specification.range *
									std::sin(halfAngle));
						};

					for (std::size_t index = 0;
						index < ringVertexCount;
						++index)
					{
						if (!boundaryIsCorrect(
							vertices[coneStart + index].position))
						{
							std::cerr << "Spot-light ring dimensions were incorrect\n";
							return EXIT_FAILURE;
						}
					}

					for (std::size_t side = 0; side < 4; ++side)
					{
						const std::size_t rayStart =
							coneStart + ringVertexCount + side * 2;

						if (!NearlyEqual(
							vertices[rayStart].position,
							specification.position) ||
							!boundaryIsCorrect(
								vertices[rayStart + 1].position))
						{
							std::cerr << "Spot-light boundary ray was incorrect\n";
							return EXIT_FAILURE;
						}
					}
				}

				const std::size_t markerStart =
					2 + coneCount * verticesPerCone;

				const std::array<glm::vec3, 3> markerAxes{
					glm::vec3{ specification.markerSize, 0.0f, 0.0f },
					glm::vec3{ 0.0f, specification.markerSize, 0.0f },
					glm::vec3{ 0.0f, 0.0f, specification.markerSize }
				};

				for (std::size_t axis = 0; axis < 3; ++axis)
				{
					if (!NearlyEqual(
						vertices[markerStart + axis * 2].position,
						specification.position - markerAxes[axis]) ||
						!NearlyEqual(
							vertices[markerStart + axis * 2 + 1].position,
							specification.position + markerAxes[axis]))
					{
						std::cerr << "Spot-light center marker was incorrect\n";
						return EXIT_FAILURE;
					}
				}
			}
		}
	}

	// Invalid spot-light specifications should fail explicitly.
	{
		using ForgeSim::Renderer::CreateWireframeSpotLightLines;
		using ForgeSim::Renderer::WireframeSpotLightSpecification;

		std::array<WireframeSpotLightSpecification, 10>
			invalidSpecifications{};

		invalidSpecifications[0].emissionDirection = glm::vec3{ 0.0f };
		invalidSpecifications[1].range = 0.0f;
		invalidSpecifications[2].innerConeAngleRadians = -0.1f;

		invalidSpecifications[3].innerConeAngleRadians =
			invalidSpecifications[3].outerConeAngleRadians;

		invalidSpecifications[4].outerConeAngleRadians =
			std::numbers::pi_v<float> *0.5f;

		invalidSpecifications[5].segmentCount = 2;
		invalidSpecifications[6].segmentCount = 4097;
		invalidSpecifications[7].markerSize = -1.0f;

		invalidSpecifications[8].position.x =
			std::numeric_limits<float>::quiet_NaN();

		invalidSpecifications[9].range =
			std::numeric_limits<float>::infinity();

		for (const auto& specification : invalidSpecifications)
		{
			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					CreateWireframeSpotLightLines(specification));
			}
			catch (const std::invalid_argument&)
			{
				exceptionWasThrown = true;
			}

			if (!exceptionWasThrown)
			{
				std::cerr << "Expected invalid spot-light input to be rejected\n";
				return EXIT_FAILURE;
			}
		}
	}

	// Directional markers add exactly three world-axis lines,
	// independently of the light direction and arrowhead setting.
	{
		using ForgeSim::Renderer::CreateDirectionalLightLines;

		for (const bool showArrowheads : { false, true })
		{
			DirectionArrowSpecification specification{
				.origin = { 1.0f, 2.0f, 3.0f },
				.direction = { 1.0f, -2.0f, 3.0f },
				.length = 2.0f,
				.color = { 0.25f, 0.5f, 0.75f },
				.showArrowheads = showArrowheads
			};

			const auto withoutMarker =
				CreateDirectionalLightLines(specification);

			specification.markerSize = 0.1f;

			const auto withMarker =
				CreateDirectionalLightLines(specification);

			if (withMarker.size() != withoutMarker.size() + 6)
			{
				std::cerr
					<< "Directional marker must add six vertices\n";
				return EXIT_FAILURE;
			}

			for (std::size_t index = 0;
				index < withoutMarker.size();
				++index)
			{
				if (!NearlyEqual(
					withMarker[index].position,
					withoutMarker[index].position) ||
					!NearlyEqual(
						withMarker[index].color,
						withoutMarker[index].color))
				{
					std::cerr
						<< "Marker changed existing directional geometry\n";
					return EXIT_FAILURE;
				}
			}

			for (int axis = 0; axis < 3; ++axis)
			{
				glm::vec3 offset{ 0.0f };
				offset[axis] = specification.markerSize;

				const std::size_t index =
					withoutMarker.size() +
					static_cast<std::size_t>(axis) * 2;

				if (!NearlyEqual(
					withMarker[index].position,
					specification.origin - offset) ||
					!NearlyEqual(
						withMarker[index + 1].position,
						specification.origin + offset) ||
					!NearlyEqual(
						withMarker[index].color,
						specification.color) ||
					!NearlyEqual(
						withMarker[index + 1].color,
						specification.color))
				{
					std::cerr
						<< "Directional marker endpoints or color incorrect\n";
					return EXIT_FAILURE;
				}
			}
		}
	}

	// Invalid directional marker sizes must be rejected.
	{
		using ForgeSim::Renderer::CreateDirectionalLightLines;

		const std::array<float, 3> invalidSizes{
			-0.1f,
			std::numeric_limits<float>::quiet_NaN(),
			std::numeric_limits<float>::infinity()
		};

		for (const float markerSize : invalidSizes)
		{
			DirectionArrowSpecification specification;
			specification.markerSize = markerSize;

			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					CreateDirectionalLightLines(specification));
			}
			catch (const std::invalid_argument&)
			{
				exceptionWasThrown = true;
			}

			if (!exceptionWasThrown)
			{
				std::cerr
					<< "Invalid directional marker size was accepted\n";
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}