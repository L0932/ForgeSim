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
					<< "Unexpected texture coordinates or "
					<< "tangents were reported\n";

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

			if (!mesh.availableAttributes.normals)
			{
				std::cerr
					<< "Expected missing normals to be generated "
					<< "for non-indexed geometry\n";

				return EXIT_FAILURE;
			}

			if (mesh.availableAttributes.textureCoordinates ||
				mesh.availableAttributes.tangents ||
				mesh.availableAttributes.colors)
			{
				std::cerr
					<< "Unexpected optional attributes were reported "
					<< "for non-indexed geometry\n";

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
						<< "Expected generated normal to point "
						<< "along positive Z\n";

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

		// The existing single-mesh fixture should also load through
		// the general scene-asset interface.
		{
			const std::filesystem::path assetPath =
				assetDirectory / "Triangle.gltf";

			const ForgeSim::Assets::SceneAsset scene =
				ForgeSim::Assets::LoadGltfScene(
					assetPath);

			if (scene.meshes.size() != 1)
			{
				std::cerr
					<< "Expected one scene mesh\n";

				return EXIT_FAILURE;
			}

			const ForgeSim::Assets::SceneMeshData& mesh =
				scene.meshes[0];

			if (mesh.primitives.size() != 1)
			{
				std::cerr
					<< "Expected one scene mesh primitive\n";

				return EXIT_FAILURE;
			}

			const ForgeSim::Assets::MeshPrimitiveData& primitive =
				mesh.primitives[0];

			if (primitive.geometry.vertices.size() != 3 ||
				primitive.geometry.indices.size() != 3)
			{
				std::cerr
					<< "Scene loader did not preserve "
					<< "the triangle geometry\n";

				return EXIT_FAILURE;
			}

			if (!scene.textureResources.HasValidReferences())
			{
				std::cerr
					<< "Scene loader produced invalid "
					<< "texture-resource references\n";

				return EXIT_FAILURE;
			}
		}

		// A complete scene should preserve roots, hierarchy, transforms,
		// meshes, primitives, materials, textures, images, and samplers.
		{
			const std::filesystem::path scenePath =
				assetDirectory / "SceneHierarchy.gltf";

			const ForgeSim::Assets::SceneAsset scene =
				ForgeSim::Assets::LoadGltfScene(
					scenePath);

			if (scene.name != "ForgeSim Test Scene")
			{
				std::cerr
					<< "Scene name was not preserved\n";

				return EXIT_FAILURE;
			}

			if (scene.rootNodeIndices.size() != 2 ||
				scene.rootNodeIndices[0] != 0 ||
				scene.rootNodeIndices[1] != 2)
			{
				std::cerr
					<< "Scene root nodes were not preserved\n";

				return EXIT_FAILURE;
			}

			if (scene.nodes.size() != 3)
			{
				std::cerr
					<< "Expected three scene nodes\n";

				return EXIT_FAILURE;
			}

			const auto& parentNode =
				scene.nodes[0];

			if (parentNode.childIndices.size() != 1 ||
				parentNode.childIndices[0] != 1 ||
				parentNode.localTransform.translation[0] != 1.0f ||
				parentNode.localTransform.translation[1] != 2.0f ||
				parentNode.localTransform.translation[2] != 3.0f)
			{
				std::cerr
					<< "Parent-node hierarchy or transform "
					<< "was not preserved\n";

				return EXIT_FAILURE;
			}

			const auto& childNode =
				scene.nodes[1];

			if (!childNode.meshIndex ||
				*childNode.meshIndex != 0 ||
				childNode.localTransform.scale[0] != 2.0f ||
				childNode.localTransform.scale[1] != 2.0f ||
				childNode.localTransform.scale[2] != 2.0f)
			{
				std::cerr
					<< "Child-node mesh or scale "
					<< "was not preserved\n";

				return EXIT_FAILURE;
			}

			const auto& matrixNode =
				scene.nodes[2];

			if (!matrixNode.meshIndex ||
				*matrixNode.meshIndex != 1 ||
				!matrixNode.localTransform.matrix ||
				(*matrixNode.localTransform.matrix)[12] != -2.0f)
			{
				std::cerr
					<< "Matrix-based node transform "
					<< "was not preserved\n";

				return EXIT_FAILURE;
			}

			if (scene.meshes.size() != 2 ||
				scene.meshes[0].primitives.size() != 2 ||
				scene.meshes[1].primitives.size() != 1)
			{
				std::cerr
					<< "Multiple meshes or primitives "
					<< "were not preserved\n";

				return EXIT_FAILURE;
			}

			const auto& firstPrimitive =
				scene.meshes[0].primitives[0];

			const auto& secondPrimitive =
				scene.meshes[0].primitives[1];

			const auto& thirdPrimitive =
				scene.meshes[1].primitives[0];

			if (!firstPrimitive.materialIndex ||
				*firstPrimitive.materialIndex != 0 ||
				!secondPrimitive.materialIndex ||
				*secondPrimitive.materialIndex != 1 ||
				!thirdPrimitive.materialIndex ||
				*thirdPrimitive.materialIndex != 1)
			{
				std::cerr
					<< "Primitive-material associations "
					<< "were not preserved\n";

				return EXIT_FAILURE;
			}

			if (scene.materials.size() != 2 ||
				!scene.materials[0]
				.metallicRoughness
				.baseColorTexture ||
				scene.materials[0]
				.metallicRoughness
				.baseColorTexture
				->textureIndex != 0 ||
				!scene.materials[1].doubleSided)
			{
				std::cerr
					<< "Scene materials were not preserved\n";

				return EXIT_FAILURE;
			}

			const auto& resources =
				scene.textureResources;

			if (resources.images.size() != 1 ||
				resources.samplers.size() != 1 ||
				resources.textures.size() != 1 ||
				!resources.HasValidReferences())
			{
				std::cerr
					<< "Scene texture resources were not loaded correctly\n";

				return EXIT_FAILURE;
			}

			if (!resources.images[0].IsValid() ||
				resources.images[0].width != 1 ||
				resources.images[0].height != 1)
			{
				std::cerr
					<< "Embedded scene image was not decoded\n";

				return EXIT_FAILURE;
			}

			const auto& texture =
				resources.textures[0];

			if (texture.imageIndex != 0 ||
				!texture.samplerIndex ||
				*texture.samplerIndex != 0)
			{
				std::cerr
					<< "Texture image or sampler reference "
					<< "was not preserved\n";

				return EXIT_FAILURE;
			}

			const auto& sampler =
				resources.samplers[0];

			if (sampler.magnificationFilter !=
				ForgeSim::Assets::TextureFilter::Linear ||
				sampler.minificationFilter !=
				ForgeSim::Assets::TextureFilter::LinearMipmapLinear ||
				sampler.wrapU !=
				ForgeSim::Assets::TextureWrap::ClampToEdge ||
				sampler.wrapV !=
				ForgeSim::Assets::TextureWrap::MirroredRepeat)
			{
				std::cerr
					<< "Sampler settings were not preserved\n";

				return EXIT_FAILURE;
			}
		}

		// Invalid scene references should produce a descriptive error.
		{
			const std::filesystem::path invalidReferencePath =
				assetDirectory /
				"InvalidSceneReference.gltf";

			bool exceptionWasThrown = false;

			try
			{
				static_cast<void>(
					ForgeSim::Assets::LoadGltfScene(
						invalidReferencePath));
			}
			catch (const std::runtime_error& exception)
			{
				exceptionWasThrown = true;

				const std::string_view message{
					exception.what()
				};

				const bool describesInvalidData =
					message.find("invalid glTF") !=
					std::string_view::npos ||
					message.find("invalid reference") !=
					std::string_view::npos;

				if (!describesInvalidData)
				{
					std::cerr
						<< "Invalid-reference error was not descriptive: "
						<< exception.what()
						<< '\n';

					return EXIT_FAILURE;
				}
			}

			if (!exceptionWasThrown)
			{
				std::cerr
					<< "Expected an invalid scene reference "
					<< "to be rejected\n";

				return EXIT_FAILURE;
			}
		}

		// UV coordinates alone must not trigger tangent generation.
		// A material without a normal texture does not require tangents.
		{
			const std::filesystem::path assetPath =
				assetDirectory / "TriangleWithoutTangents.gltf";

			const ForgeSim::Assets::MeshData mesh =
				ForgeSim::Assets::LoadGltfMesh(assetPath);

			if (mesh.vertices.size() != 3 ||
				mesh.indices.size() != 3)
			{
				std::cerr
					<< "Expected three vertices and three indices\n";

				return EXIT_FAILURE;
			}

			if (!mesh.availableAttributes.normals ||
				!mesh.availableAttributes.textureCoordinates ||
				mesh.availableAttributes.tangents)
			{
				std::cerr
					<< "Expected normals and UVs without generated tangents "
					<< "for a material without a normal texture\n";

				return EXIT_FAILURE;
			}

			const ForgeSim::Assets::SceneAsset scene =
				ForgeSim::Assets::LoadGltfScene(assetPath);

			if (scene.meshes.size() != 1 ||
				scene.meshes[0].primitives.size() != 1 ||
				scene.materials.size() != 1)
			{
				std::cerr
					<< "Unexpected scene structure in no-normal-map fixture\n";

				return EXIT_FAILURE;
			}

			const auto& primitive =
				scene.meshes[0].primitives[0];

			if (!primitive.materialIndex ||
				*primitive.materialIndex != 0 ||
				scene.materials[0].normalTexture)
			{
				std::cerr
					<< "Expected an assigned material without a normal texture\n";

				return EXIT_FAILURE;
			}

			if (!primitive.geometry.availableAttributes.normals ||
				!primitive.geometry.availableAttributes.textureCoordinates ||
				primitive.geometry.availableAttributes.tangents)
			{
				std::cerr
					<< "Scene loading generated unnecessary tangents\n";

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