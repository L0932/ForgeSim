#pragma once

#include <vector>
#include <numbers>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Renderer
{
	struct DirectionArrowSpecification
	{
		glm::vec3 origin{ 0.0f };

		// World-space direction in which the arrow points.
		// Does not need to be normalized.
		glm::vec3 direction{ 0.0f, -1.0f, 0.0f };

		float length = 1.0f;
		glm::vec3 color{ 1.0f };

		bool showArrowheads = true;
	};

	struct WireframeSphereSpecification
	{
		glm::vec3 center{ 0.0f };
		float radius = 1.0f;
		int segmentCount = 32;

		// Half-length of each central marker axis, in world units.
		float markerSize = 0.05f;

		glm::vec3 color{ 1.0f };
	};

	struct WireframeSpotLightSpecification
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 emissionDirection{ 0.0f, -1.0f, 0.0f };

		float range = 1.0f;
		float innerConeAngleRadians = 0.2617994f;
		float outerConeAngleRadians = 0.5235988f;

		int segmentCount = 32;
		float markerSize = 0.05f;

		glm::vec3 color{ 1.0f };
	};

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

	[[nodiscard]] std::vector<DebugLineVertex>
		CreateDirectionArrowLines(
			const DirectionArrowSpecification& specification);

	// A ring with parallel arrows indicating light travel.
	// Uses origin, direction, length, and color from the specification.
	[[nodiscard]] std::vector<DebugLineVertex>
		CreateDirectionalLightLines(
			const DirectionArrowSpecification& specification);

	[[nodiscard]] std::vector<DebugLineVertex>
		CreateWireframeSphereLines(
			const WireframeSphereSpecification& specification);

	[[nodiscard]] std::vector<DebugLineVertex>
		CreateWireframeSpotLightLines(
			const WireframeSpotLightSpecification& specification);
}