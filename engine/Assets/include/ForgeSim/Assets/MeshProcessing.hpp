#pragma once

#include <ForgeSim/Assets/MeshData.hpp>

namespace ForgeSim::Assets
{
	// Generates missing geometric attributes when sufficient mesh data
	// is available. Authored attributes are preserved. Generated tangent
	// handedness follows glTF UVs (top-left) and normal-map +Y (up).
	void GenerateMissingNormalsAndTangents(
		MeshData& mesh,
		bool generateTangents);
}