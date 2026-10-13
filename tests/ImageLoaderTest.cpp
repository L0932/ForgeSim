#include <ForgeSim/Assets/ImageLoader.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main()
{
	// A 2x2 RGBA PNG containing red, green, blue, and white pixels.
	constexpr std::array<std::uint8_t, 82> pngData{
		0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
		0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
		0x08, 0x06, 0x00, 0x00, 0x00, 0x72, 0xB6, 0x0D,
		0x24, 0x00, 0x00, 0x00, 0x19, 0x49, 0x44, 0x41,
		0x54, 0x78, 0xDA, 0x05, 0xC1, 0x01, 0x0D, 0x00,
		0x00, 0x0C, 0xC3, 0x20, 0x96, 0xDC, 0xBF, 0xE5,
		0x1E, 0x44, 0xD2, 0x4D, 0xC2, 0x03, 0x3E, 0xFF,
		0x06, 0x00, 0x13, 0x31, 0x3B, 0x79, 0x00, 0x00,
		0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42,
		0x60, 0x82
	};

	// Valid PNG data should decode into the documented RGBA8 layout.
	{
		const ForgeSim::Assets::ImageData image =
			ForgeSim::Assets::DecodeImage(
				pngData,
				"embedded test PNG");

		if (!image.IsValid())
		{
			std::cerr
				<< "Decoded image was not valid.\n";

			return EXIT_FAILURE;
		}

		if (image.width != 2 ||
			image.height != 2 ||
			image.channelCount != 4)
		{
			std::cerr
				<< "Decoded image had unexpected dimensions "
				<< "or channel count.\n";

			return EXIT_FAILURE;
		}

		constexpr std::array<std::uint8_t, 16>
			expectedPixels{
				255, 0, 0, 255,
				0, 255, 0, 255,
				0, 0, 255, 255,
				255, 255, 255, 255
		};

		if (!std::equal(
			image.pixels.begin(),
			image.pixels.end(),
			expectedPixels.begin(),
			expectedPixels.end()))
		{
			std::cerr
				<< "Decoded pixels did not match "
				<< "the expected RGBA8 values.\n";

			return EXIT_FAILURE;
		}
	}

	// Empty encoded data should fail descriptively.
	{
		bool exceptionWasThrown = false;

		try
		{
			static_cast<void>(
				ForgeSim::Assets::DecodeImage(
					{},
					"empty test image"));
		}
		catch (const std::runtime_error& exception)
		{
			exceptionWasThrown = true;

			const std::string_view message{
				exception.what()
			};

			if (message.find("empty") ==
				std::string_view::npos)
			{
				std::cerr
					<< "Empty-image error was not descriptive: "
					<< exception.what()
					<< '\n';

				return EXIT_FAILURE;
			}
		}

		if (!exceptionWasThrown)
		{
			std::cerr
				<< "Expected empty image data to be rejected.\n";

			return EXIT_FAILURE;
		}
	}

	// Invalid encoded bytes should fail descriptively.
	{
		constexpr std::array<std::uint8_t, 4>
			invalidData{
				0x00,
				0x01,
				0x02,
				0x03
		};

		bool exceptionWasThrown = false;

		try
		{
			static_cast<void>(
				ForgeSim::Assets::DecodeImage(
					invalidData,
					"invalid test image"));
		}
		catch (const std::runtime_error& exception)
		{
			exceptionWasThrown = true;

			const std::string_view message{
				exception.what()
			};

			if (message.find("Failed to decode image") ==
				std::string_view::npos)
			{
				std::cerr
					<< "Invalid-image error was not descriptive: "
					<< exception.what()
					<< '\n';

				return EXIT_FAILURE;
			}
		}

		if (!exceptionWasThrown)
		{
			std::cerr
				<< "Expected invalid image data to be rejected.\n";

			return EXIT_FAILURE;
		}
	}

	// Texture resources should accept valid image and sampler references.
	{
		ForgeSim::Assets::TextureResources resources;

		resources.images.push_back(
			ForgeSim::Assets::ImageData{
				.width = 1,
				.height = 1,
				.channelCount = 4,
				.pixels = {
					255,
					255,
					255,
					255
				}
			});

		resources.samplers.push_back(
			ForgeSim::Assets::SamplerData{});

		resources.textures.push_back(
			ForgeSim::Assets::TextureData{
				.imageIndex = 0,
				.samplerIndex = 0
			});

		if (!resources.HasValidReferences())
		{
			std::cerr
				<< "Valid texture resource references "
				<< "were rejected.\n";

			return EXIT_FAILURE;
		}
	}

	// An omitted sampler should use default sampling behavior.
	{
		ForgeSim::Assets::TextureResources resources;

		resources.images.push_back(
			ForgeSim::Assets::ImageData{
				.width = 1,
				.height = 1,
				.channelCount = 4,
				.pixels = {
					255,
					255,
					255,
					255
				}
			});

		resources.textures.push_back(
			ForgeSim::Assets::TextureData{
				.imageIndex = 0,
				.samplerIndex = std::nullopt
			});

		if (!resources.HasValidReferences())
		{
			std::cerr
				<< "A texture without an explicit sampler "
				<< "was rejected.\n";

			return EXIT_FAILURE;
		}
	}

	// Out-of-range image references should be rejected.
	{
		ForgeSim::Assets::TextureResources resources;

		resources.textures.push_back(
			ForgeSim::Assets::TextureData{
				.imageIndex = 0,
				.samplerIndex = std::nullopt
			});

		if (resources.HasValidReferences())
		{
			std::cerr
				<< "An invalid image reference "
				<< "was accepted.\n";

			return EXIT_FAILURE;
		}
	}

	// Out-of-range sampler references should be rejected.
	{
		ForgeSim::Assets::TextureResources resources;

		resources.images.push_back(
			ForgeSim::Assets::ImageData{
				.width = 1,
				.height = 1,
				.channelCount = 4,
				.pixels = {
					255,
					255,
					255,
					255
				}
			});

		resources.textures.push_back(
			ForgeSim::Assets::TextureData{
				.imageIndex = 0,
				.samplerIndex = 0
			});

		if (resources.HasValidReferences())
		{
			std::cerr
				<< "An invalid sampler reference "
				<< "was accepted.\n";

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}