#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Renderer
{
	struct DebugLineVertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	struct ReferenceGridSpecification
	{
		int halfLineCount = 10;
		float spacing = 1.0f;
		float height = -0.75f;

		glm::vec3 lineColor{
			0.35f,
			0.35f,
			0.35f
		};

		glm::vec3 xAxisColor{
			0.9f,
			0.2f,
			0.2f
		};

		glm::vec3 zAxisColor{
			0.2f,
			0.4f,
			0.9f
		};
	};

	[[nodiscard]] std::vector<DebugLineVertex>
		CreateReferenceGridLines(
			const ReferenceGridSpecification& specification);

	[[nodiscard]] std::vector<DebugLineVertex>
		CreateWireframeBoxLines(
			const AxisAlignedBoundingBox& localBounds,
			const glm::mat4& modelMatrix,
			const glm::vec3& color);
}