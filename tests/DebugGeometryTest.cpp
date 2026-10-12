#include <ForgeSim/Renderer/DebugGeometry.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <glm/common.hpp>
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

	return EXIT_SUCCESS;
}