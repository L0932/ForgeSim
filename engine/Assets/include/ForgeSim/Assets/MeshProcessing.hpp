#pragma once

#include <ForgeSim/Assets/MeshData.hpp>

namespace ForgeSim::Assets
{
	// Generates missing geometric attributes when sufficient mesh data
	// is available. Authored attributes are preserved.
	void GenerateMissingNormalsAndTangents(
		MeshData& mesh);
}