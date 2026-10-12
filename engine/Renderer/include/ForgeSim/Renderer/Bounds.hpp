#pragma once

#include <glm/vec3.hpp>

namespace ForgeSim::Renderer
{
	struct AxisAlignedBoundingBox
	{
		glm::vec3 minimum;
		glm::vec3 maximum;
	};
}