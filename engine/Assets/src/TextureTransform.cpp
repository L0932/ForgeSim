#include <ForgeSim/Assets/TextureTransform.hpp>

#include <cmath>

namespace ForgeSim::Assets
{
	bool IsFiniteTextureTransform(const TextureTransform& transform) noexcept
	{
		return std::isfinite(transform.offset[0]) &&
			std::isfinite(transform.offset[1]) &&
			std::isfinite(transform.rotationRadians) &&
			std::isfinite(transform.scale[0]) &&
			std::isfinite(transform.scale[1]);
	}

	TextureTransformRows CalculateTextureTransformRows(
		const TextureTransform& transform) noexcept
	{
		const float cosine = std::cos(transform.rotationRadians);
		const float sine = std::sin(transform.rotationRadians);
		return {{
			{ cosine * transform.scale[0], -sine * transform.scale[1], transform.offset[0] },
			{ sine * transform.scale[0], cosine * transform.scale[1], transform.offset[1] }
		}};
	}

	std::array<float, 2> ApplyTextureTransform(
		const TextureTransform& transform,
		const std::array<float, 2>& coordinates) noexcept
	{
		const auto rows = CalculateTextureTransformRows(transform);
		return {
			rows[0][0] * coordinates[0] + rows[0][1] * coordinates[1] + rows[0][2],
			rows[1][0] * coordinates[0] + rows[1][1] * coordinates[1] + rows[1][2]
		};
	}
}
