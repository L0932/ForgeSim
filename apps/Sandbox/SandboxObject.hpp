#pragma once

#include "Transform.hpp"
#include <ForgeSim/Renderer/Bounds.hpp>
#include <optional>

#include <cstdint>
#include <string>

namespace ForgeSim::Sandbox
{
	using SandboxObjectId = std::uint64_t;

	struct SandboxObject
	{
		SandboxObjectId id;
		Transform transform;

		std::string name = "Object";

		// Project-relative UTF-8 model reference.
		std::string modelPath;
        // Runtime model bounds; reconstructed when loading resources.
        std::optional<Renderer::AxisAlignedBoundingBox> localBounds = std::nullopt;
	};
}