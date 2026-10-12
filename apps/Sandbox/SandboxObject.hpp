#pragma once

#include "Transform.hpp"

#include <cstdint>

#include <ForgeSim/Renderer/Bounds.hpp>

namespace ForgeSim::Sandbox
{
	using SandboxObjectId = std::uint64_t;

	struct SandboxObject
	{
		SandboxObjectId id;
		Transform transform;
	};
}