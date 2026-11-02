#pragma once

#include <array>

namespace ForgeSim::Assets
{
	// glTF KHR_texture_transform: scale, then rotate around the UV origin,
	// then translate. Values apply to one texture usage, not to mesh data.
	struct TextureTransform
	{
		std::array<float, 2> offset{ 0.0f, 0.0f };
		float rotationRadians = 0.0f;
		std::array<float, 2> scale{ 1.0f, 1.0f };
	};

	[[nodiscard]] bool IsFiniteTextureTransform(
		const TextureTransform& transform) noexcept;

	// Two affine rows: (a, b, offsetU), (c, d, offsetV).
	// Each output coordinate is dot(row, vec3(uv, 1)).
	using TextureTransformRows = std::array<std::array<float, 3>, 2>;

	[[nodiscard]] TextureTransformRows CalculateTextureTransformRows(
		const TextureTransform& transform) noexcept;

	// CPU reference operation for tooling and headless tests. Does not wrap,
	// clamp, flip V, or modify the source coordinates. Samplers own wrapping.
	[[nodiscard]] std::array<float, 2> ApplyTextureTransform(
		const TextureTransform& transform,
		const std::array<float, 2>& coordinates) noexcept;
}
