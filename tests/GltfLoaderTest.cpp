#include <ForgeSim/Assets/GltfLoader.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main()
{
	try
	{
		const std::filesystem::path assetDirectory{
			FORGESIM_TEST_ASSET_DIRECTORY
		};

		// Missing files should produce a descriptive exception.
		{
			const std::filesystem::path missingPath =
				assetDirectory / "MissingModel.gltf";

			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					ForgeSim::Assets::LoadGltfMesh(
						missingPath));
			}
			catch (const std::runtime_error& exception)
			{
				exceptionWasThrown = true;

				const std::string_view message{
					exception.what()
				};

				if (message.find("file not found") ==
					std::string_view::npos)
				{
					std::cerr
						<< "Missing-file error was not descriptive: "
						<< exception.what()
						<< '\n';

					return EXIT_FAILURE;
				}
			}

			if (!exceptionWasThrown)
			{
				std::cerr
					<< "Expected a missing glTF file to be rejected\n";

				return EXIT_FAILURE;
			}
		}

		// Indexed geometry should preserve vertex data and indices.
		{
			const std::filesystem::path assetPath =
				assetDirectory / "Triangle.gltf";

			const ForgeSim::Assets::MeshData mesh =
				ForgeSim::Assets::LoadGltfMesh(assetPath);

			if (mesh.vertices.size() != 3)
			{
				std::cerr << "Expected three vertices\n";
				return EXIT_FAILURE;
			}

			if (mesh.indices.size() != 3)
			{
				std::cerr << "Expected three indices\n";
				return EXIT_FAILURE;
			}

			if (mesh.indices[0] != 0 ||
				mesh.indices[1] != 1 ||
				mesh.indices[2] != 2)
			{
				std::cerr << "Unexpected triangle indices\n";
				return EXIT_FAILURE;
			}

			const auto& firstVertex = mesh.vertices[0];

			if (firstVertex.position[0] != -0.5f ||
				firstVertex.position[1] != -0.5f ||
				firstVertex.position[2] != 0.0f)
			{
				std::cerr
					<< "Unexpected first vertex position\n";

				return EXIT_FAILURE;
			}

			if (firstVertex.color[0] != 1.0f ||
				firstVertex.color[1] != 0.0f ||
				firstVertex.color[2] != 0.0f)
			{
				std::cerr
					<< "Unexpected first vertex color\n";

				return EXIT_FAILURE;
			}

			if (!mesh.availableAttributes.colors)
			{
				std::cerr
					<< "Expected COLOR_0 to be reported as available\n";

				return EXIT_FAILURE;
			}

			if (mesh.availableAttributes.normals ||
				mesh.availableAttributes.textureCoordinates ||
				mesh.availableAttributes.tangents)
			{
				std::cerr
					<< "Unexpected optional attributes were reported\n";

				return EXIT_FAILURE;
			}
		}

		// Interleaved vertex data should preserve normals,
		// texture coordinates, tangents, colors, and indices.
		{
			const std::filesystem::path assetPath =
				assetDirectory /
				"TriangleWithAttributes.gltf";

			const ForgeSim::Assets::MeshData mesh =
				ForgeSim::Assets::LoadGltfMesh(
					assetPath);

			if (mesh.vertices.size() != 3 ||
				mesh.indices.size() != 3)
			{
				std::cerr
					<< "Expected an indexed triangle with "
					<< "three vertices\n";

				return EXIT_FAILURE;
			}

			if (!mesh.availableAttributes.normals ||
				!mesh.availableAttributes.textureCoordinates ||
				!mesh.availableAttributes.tangents ||
				!mesh.availableAttributes.colors)
			{
				std::cerr
					<< "Expected all optional vertex attributes "
					<< "to be reported as available\n";

				return EXIT_FAILURE;
			}

			if (mesh.indices[0] != 0 ||
				mesh.indices[1] != 1 ||
				mesh.indices[2] != 2)
			{
				std::cerr
					<< "Unexpected interleaved triangle indices\n";

				return EXIT_FAILURE;
			}

			for (const auto& vertex : mesh.vertices)
			{
				if (vertex.normal[0] != 0.0f ||
					vertex.normal[1] != 0.0f ||
					vertex.normal[2] != 1.0f)
				{
					std::cerr
						<< "Unexpected imported normal\n";

					return EXIT_FAILURE;
				}

				if (vertex.tangent[0] != 1.0f ||
					vertex.tangent[1] != 0.0f ||
					vertex.tangent[2] != 0.0f ||
					vertex.tangent[3] != 1.0f)
				{
					std::cerr
						<< "Unexpected imported tangent\n";

					return EXIT_FAILURE;
				}
			}

			const auto& firstVertex = mesh.vertices[0];
			const auto& secondVertex = mesh.vertices[1];
			const auto& thirdVertex = mesh.vertices[2];

			if (firstVertex.textureCoordinates[0] != 0.0f ||
				firstVertex.textureCoordinates[1] != 0.0f ||
				secondVertex.textureCoordinates[0] != 1.0f ||
				secondVertex.textureCoordinates[1] != 0.0f ||
				thirdVertex.textureCoordinates[0] != 0.5f ||
				thirdVertex.textureCoordinates[1] != 1.0f)
			{
				std::cerr
					<< "Unexpected imported texture coordinates\n";

				return EXIT_FAILURE;
			}

			if (firstVertex.color[0] != 1.0f ||
				firstVertex.color[1] != 0.0f ||
				firstVertex.color[2] != 0.0f ||
				secondVertex.color[0] != 0.0f ||
				secondVertex.color[1] != 1.0f ||
				secondVertex.color[2] != 0.0f ||
				thirdVertex.color[0] != 0.0f ||
				thirdVertex.color[1] != 0.0f ||
				thirdVertex.color[2] != 1.0f)
			{
				std::cerr
					<< "Unexpected imported vertex colors\n";

				return EXIT_FAILURE;
			}
		}

		// Non-indexed geometry should receive sequential indices,
		// and missing colors should default to white.
		{
			const std::filesystem::path nonIndexedPath =
				assetDirectory /
				"TriangleWithoutIndices.gltf";

			const ForgeSim::Assets::MeshData mesh =
				ForgeSim::Assets::LoadGltfMesh(
					nonIndexedPath);

			if (mesh.vertices.size() != 3)
			{
				std::cerr
					<< "Expected three non-indexed vertices\n";

				return EXIT_FAILURE;
			}

			if (mesh.indices.size() != 3 ||
				mesh.indices[0] != 0 ||
				mesh.indices[1] != 1 ||
				mesh.indices[2] != 2)
			{
				std::cerr
					<< "Expected generated sequential indices\n";

				return EXIT_FAILURE;
			}

			if (mesh.availableAttributes.normals ||
				mesh.availableAttributes.textureCoordinates ||
				mesh.availableAttributes.tangents ||
				mesh.availableAttributes.colors)
			{
				std::cerr
					<< "Missing optional attributes were incorrectly "
					<< "reported as available\n";

				return EXIT_FAILURE;
			}

			for (const auto& vertex : mesh.vertices)
			{
				if (vertex.color[0] != 1.0f ||
					vertex.color[1] != 1.0f ||
					vertex.color[2] != 1.0f)
				{
					std::cerr
						<< "Expected missing vertex colors "
						<< "to default to white\n";

					return EXIT_FAILURE;
				}
			}

			for (const auto& vertex : mesh.vertices)
			{
				if (vertex.normal[0] != 0.0f ||
					vertex.normal[1] != 0.0f ||
					vertex.normal[2] != 1.0f)
				{
					std::cerr
						<< "Expected missing normals to default "
						<< "to positive Z\n";

					return EXIT_FAILURE;
				}

				if (vertex.textureCoordinates[0] != 0.0f ||
					vertex.textureCoordinates[1] != 0.0f)
				{
					std::cerr
						<< "Expected missing texture coordinates "
						<< "to default to zero\n";

					return EXIT_FAILURE;
				}

				if (vertex.tangent[0] != 1.0f ||
					vertex.tangent[1] != 0.0f ||
					vertex.tangent[2] != 0.0f ||
					vertex.tangent[3] != 1.0f)
				{
					std::cerr
						<< "Expected missing tangents to use "
						<< "the fallback tangent\n";

					return EXIT_FAILURE;
				}

				if (vertex.color[0] != 1.0f ||
					vertex.color[1] != 1.0f ||
					vertex.color[2] != 1.0f)
				{
					std::cerr
						<< "Expected missing vertex colors "
						<< "to default to white\n";

					return EXIT_FAILURE;
				}
			}
		}
		
		// Malformed glTF data should fail during parsing.
		{
			const std::filesystem::path invalidPath =
				assetDirectory / "Invalid.gltf";

			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					ForgeSim::Assets::LoadGltfMesh(
						invalidPath));
			}
			catch (const std::runtime_error& exception)
			{
				exceptionWasThrown = true;

				const std::string_view message{
					exception.what()
				};

				if (message.find("Failed to parse glTF file") ==
					std::string_view::npos)
				{
					std::cerr
						<< "Malformed-file error was not descriptive: "
						<< exception.what()
						<< '\n';

					return EXIT_FAILURE;
				}
			}

			if (!exceptionWasThrown)
			{
				std::cerr
					<< "Expected malformed glTF data to be rejected\n";

				return EXIT_FAILURE;
			}
		}

		// Unsupported primitive modes should fail explicitly.
		{
			const std::filesystem::path linesPath =
				assetDirectory / "UnsupportedLines.gltf";

			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					ForgeSim::Assets::LoadGltfMesh(
						linesPath));
			}
			catch (const std::runtime_error& exception)
			{
				exceptionWasThrown = true;

				const std::string_view message{
					exception.what()
				};

				if (message.find("not composed of triangles") ==
					std::string_view::npos)
				{
					std::cerr
						<< "Unsupported-primitive error was not descriptive: "
						<< exception.what()
						<< '\n';

					return EXIT_FAILURE;
				}
			}

			if (!exceptionWasThrown)
			{
				std::cerr
					<< "Expected a line primitive to be rejected\n";

				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}