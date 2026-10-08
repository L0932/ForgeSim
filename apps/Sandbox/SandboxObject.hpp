#pragma once

#include "Transform.hpp"

#include <cstdint>

namespace ForgeSim::Sandbox
{
	using SandboxObjectId = std::uint64_t;

	struct SandboxObject
	{
		SandboxObjectId id;
		Transform transform;
	};
}