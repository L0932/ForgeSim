#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>

#include <ForgeSim/Assets/TextureData.hpp>

namespace ForgeSim::Assets
{
	[[nodiscard]] ImageData LoadImage(
		const std::filesystem::path& path);

	[[nodiscard]] ImageData DecodeImage(
		std::span<const std::uint8_t> encodedData,
		std::string_view sourceName = "<memory>");
}