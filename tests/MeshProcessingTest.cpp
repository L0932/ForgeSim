#include <ForgeSim/Assets/MeshProcessing.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace
{
	[[nodiscard]] bool NearlyEqual(
		float left,
		float right,
		float tolerance = 1e-5f) noexcept
	{
		return std::abs(left - right) <= tolerance;
	}
}

int main()
{
	using ForgeSim::Assets::GenerateMissingNormalsAndTangents;
	using ForgeSim::Assets::MeshData;
	using ForgeSim::Assets::MeshVertex;

	// Two indexed triangles sharing vertices should generate
	// consistent smooth normals.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { -1.0f, -1.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, -1.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 1.0f, 0.0f }
			},
			MeshVertex{
				.position = { -1.0f, 1.0f, 0.0f }
			}
		};

		mesh.indices = {
			0, 1, 2,
			0, 2, 3
		};

		GenerateMissingNormalsAndTangents(mesh);

		if (!mesh.availableAttributes.normals)
		{
			std::cerr
				<< "Expected missing normals to be generated\n";

			return EXIT_FAILURE;
		}

		if (mesh.availableAttributes.textureCoordinates ||
			mesh.availableAttributes.tangents)
		{
			std::cerr
				<< "Unexpected texture-coordinate or tangent data\n";

			return EXIT_FAILURE;
		}

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.normal[0], 0.0f) ||
				!NearlyEqual(vertex.normal[1], 0.0f) ||
				!NearlyEqual(vertex.normal[2], 1.0f))
			{
				std::cerr
					<< "Generated normal had an unexpected value\n";

				return EXIT_FAILURE;
			}

			const float length =
				std::sqrt(
					vertex.normal[0] * vertex.normal[0] +
					vertex.normal[1] * vertex.normal[1] +
					vertex.normal[2] * vertex.normal[2]);

			if (!NearlyEqual(length, 1.0f))
			{
				std::cerr
					<< "Generated normal was not normalized\n";

				return EXIT_FAILURE;
			}
		}
	}

	// Authored normals must remain unchanged.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 1.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 1.0f, 0.0f }
			},
			MeshVertex{
				.position = { 0.0f, 1.0f, 0.0f },
				.normal = { 0.0f, 1.0f, 0.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };
		mesh.availableAttributes.normals = true;

		GenerateMissingNormalsAndTangents(mesh);

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.normal[0], 0.0f) ||
				!NearlyEqual(vertex.normal[1], 1.0f) ||
				!NearlyEqual(vertex.normal[2], 0.0f))
			{
				std::cerr
					<< "Authored normal was modified\n";

				return EXIT_FAILURE;
			}
		}
	}

	// Fully degenerate geometry should fail descriptively
	// instead of producing invalid normals.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 2.0f, 0.0f, 0.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };

		bool exceptionWasThrown = false;

		try
		{
			GenerateMissingNormalsAndTangents(mesh);
		}
		catch (const std::runtime_error& exception)
		{
			exceptionWasThrown = true;

			const std::string_view message{
				exception.what()
			};

			if (message.find("zero-length") ==
				std::string_view::npos)
			{
				std::cerr
					<< "Degenerate-geometry error was not descriptive: "
					<< exception.what()
					<< '\n';

				return EXIT_FAILURE;
			}
		}

		if (!exceptionWasThrown)
		{
			std::cerr
				<< "Expected degenerate geometry to be rejected\n";

			return EXIT_FAILURE;
		}
	}

	// A textured indexed quad should generate normalized tangents
	// with valid handedness.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { -1.0f, -1.0f, 0.0f },
				.textureCoordinates = { 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, -1.0f, 0.0f },
				.textureCoordinates = { 1.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 1.0f, 0.0f },
				.textureCoordinates = { 1.0f, 1.0f }
			},
			MeshVertex{
				.position = { -1.0f, 1.0f, 0.0f },
				.textureCoordinates = { 0.0f, 1.0f }
			}
		};

		mesh.indices = {
			0, 1, 2,
			0, 2, 3
		};

		mesh.availableAttributes.textureCoordinates =
			true;

		GenerateMissingNormalsAndTangents(mesh);

		if (!mesh.availableAttributes.normals ||
			!mesh.availableAttributes.tangents)
		{
			std::cerr
				<< "Generated normal or tangent data "
				<< "was not marked available\n";

			return EXIT_FAILURE;
		}

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.tangent[0], 1.0f) ||
				!NearlyEqual(vertex.tangent[1], 0.0f) ||
				!NearlyEqual(vertex.tangent[2], 0.0f))
			{
				std::cerr
					<< "Generated tangent had an "
					<< "unexpected direction\n";

				return EXIT_FAILURE;
			}

			const float tangentLength =
				std::sqrt(
					vertex.tangent[0] *
					vertex.tangent[0] +
					vertex.tangent[1] *
					vertex.tangent[1] +
					vertex.tangent[2] *
					vertex.tangent[2]);

			if (!NearlyEqual(tangentLength, 1.0f))
			{
				std::cerr
					<< "Generated tangent was not normalized\n";

				return EXIT_FAILURE;
			}

			const float normalDotTangent =
				vertex.normal[0] * vertex.tangent[0] +
				vertex.normal[1] * vertex.tangent[1] +
				vertex.normal[2] * vertex.tangent[2];

			if (!NearlyEqual(normalDotTangent, 0.0f))
			{
				std::cerr
					<< "Generated tangent was not "
					<< "orthogonal to its normal\n";

				return EXIT_FAILURE;
			}

			if (!NearlyEqual(
				std::abs(vertex.tangent[3]),
				1.0f))
			{
				std::cerr
					<< "Generated tangent had invalid handedness\n";

				return EXIT_FAILURE;
			}
		}
	}

	// Authored tangents must remain unchanged.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 0.0f },
				.tangent = { 0.0f, 1.0f, 0.0f, -1.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 1.0f, 0.0f },
				.tangent = { 0.0f, 1.0f, 0.0f, -1.0f }
			},
			MeshVertex{
				.position = { 0.0f, 1.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 1.0f },
				.tangent = { 0.0f, 1.0f, 0.0f, -1.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };

		mesh.availableAttributes.normals = true;
		mesh.availableAttributes.textureCoordinates = true;
		mesh.availableAttributes.tangents = true;

		GenerateMissingNormalsAndTangents(mesh);

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.tangent[0], 0.0f) ||
				!NearlyEqual(vertex.tangent[1], 1.0f) ||
				!NearlyEqual(vertex.tangent[2], 0.0f) ||
				!NearlyEqual(vertex.tangent[3], -1.0f))
			{
				std::cerr
					<< "Authored tangent was modified\n";

				return EXIT_FAILURE;
			}
		}
	}

	// Meshes without texture coordinates should retain the
	// fallback tangent and remain marked as lacking tangents.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 0.0f, 1.0f, 0.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };

		GenerateMissingNormalsAndTangents(mesh);

		if (mesh.availableAttributes.tangents)
		{
			std::cerr
				<< "Tangents were reported without "
				<< "texture coordinates\n";

			return EXIT_FAILURE;
		}

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.tangent[0], 1.0f) ||
				!NearlyEqual(vertex.tangent[1], 0.0f) ||
				!NearlyEqual(vertex.tangent[2], 0.0f) ||
				!NearlyEqual(vertex.tangent[3], 1.0f))
			{
				std::cerr
					<< "Fallback tangent was unexpectedly modified\n";

				return EXIT_FAILURE;
			}
		}
	}

	// Degenerate texture-coordinate gradients must fail
	// without producing invalid tangent values.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 0.0f, 1.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 0.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };

		mesh.availableAttributes.normals = true;
		mesh.availableAttributes.textureCoordinates = true;

		bool exceptionWasThrown = false;

		try
		{
			GenerateMissingNormalsAndTangents(mesh);
		}
		catch (const std::runtime_error& exception)
		{
			exceptionWasThrown = true;

			const std::string_view message{
				exception.what()
			};

			if (message.find("zero-length") ==
				std::string_view::npos &&
				message.find("degenerate") ==
				std::string_view::npos)
			{
				std::cerr
					<< "Degenerate-UV error was not descriptive: "
					<< exception.what()
					<< '\n';

				return EXIT_FAILURE;
			}
		}

		if (!exceptionWasThrown)
		{
			std::cerr
				<< "Expected degenerate texture coordinates "
				<< "to prevent tangent generation\n";

			return EXIT_FAILURE;
		}

		for (const MeshVertex& vertex : mesh.vertices)
		{
			for (const float component : vertex.tangent)
			{
				if (!std::isfinite(component))
				{
					std::cerr
						<< "Degenerate input produced a "
						<< "non-finite tangent\n";

					return EXIT_FAILURE;
				}
			}
		}
	}

	// Mirrored texture coordinates should produce negative
	// tangent-space handedness.
	{
		MeshData mesh;

		mesh.vertices = {
			MeshVertex{
				.position = { 0.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 0.0f }
			},
			MeshVertex{
				.position = { 1.0f, 0.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 0.0f, 1.0f }
			},
			MeshVertex{
				.position = { 0.0f, 1.0f, 0.0f },
				.normal = { 0.0f, 0.0f, 1.0f },
				.textureCoordinates = { 1.0f, 0.0f }
			}
		};

		mesh.indices = { 0, 1, 2 };

		mesh.availableAttributes.normals = true;
		mesh.availableAttributes.textureCoordinates = true;

		GenerateMissingNormalsAndTangents(mesh);

		if (!mesh.availableAttributes.tangents)
		{
			std::cerr
				<< "Mirrored UVs did not generate tangents\n";

			return EXIT_FAILURE;
		}

		for (const MeshVertex& vertex : mesh.vertices)
		{
			if (!NearlyEqual(vertex.tangent[0], 0.0f) ||
				!NearlyEqual(vertex.tangent[1], 1.0f) ||
				!NearlyEqual(vertex.tangent[2], 0.0f) ||
				!NearlyEqual(vertex.tangent[3], -1.0f))
			{
				std::cerr
					<< "Mirrored UV tangent had an "
					<< "incorrect direction or handedness\n";

				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}