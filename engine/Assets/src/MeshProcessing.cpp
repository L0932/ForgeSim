#include <ForgeSim/Assets/MeshProcessing.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace ForgeSim::Assets
{
	namespace
	{
		[[nodiscard]] glm::vec3 ToVector3(
			const std::array<float, 3>& value) noexcept
		{
			return {
				value[0],
				value[1],
				value[2]
			};
		}

		[[nodiscard]] glm::vec2 ToVector2(
			const std::array<float, 2>& value) noexcept
		{
			return {
				value[0],
				value[1]
			};
		}

		[[nodiscard]] std::array<float, 3> ToArray(
			const glm::vec3& value) noexcept
		{
			return {
				value.x,
				value.y,
				value.z
			};
		}

		[[nodiscard]] bool IsFinite(
			const glm::vec3& value) noexcept
		{
			return
				std::isfinite(value.x) &&
				std::isfinite(value.y) &&
				std::isfinite(value.z);
		}

		[[nodiscard]] glm::vec3 NormalizeGenerated(
			const glm::vec3& value)
		{
			const float lengthSquared =
				glm::dot(value, value);

			constexpr float minimumLengthSquared =
				1e-12f;

			if (!std::isfinite(lengthSquared) ||
				lengthSquared <= minimumLengthSquared)
			{
				throw std::runtime_error(
					"Cannot normalize a zero-length or "
					"non-finite generated mesh attribute.");
			}

			return value / std::sqrt(lengthSquared);
		}

		void GenerateSmoothNormals(MeshData& mesh)
		{
			if (mesh.availableAttributes.normals)
			{
				return;
			}

			std::vector<glm::vec3> accumulatedNormals(
				mesh.vertices.size(),
				glm::vec3{ 0.0f });

			constexpr float minimumFaceLengthSquared =
				1e-12f;

			for (std::size_t index = 0;
				index < mesh.indices.size();
				index += 3)
			{
				const std::uint32_t firstIndex =
					mesh.indices[index];

				const std::uint32_t secondIndex =
					mesh.indices[index + 1];

				const std::uint32_t thirdIndex =
					mesh.indices[index + 2];

				const glm::vec3 firstPosition =
					ToVector3(
						mesh.vertices[firstIndex].position);

				const glm::vec3 secondPosition =
					ToVector3(
						mesh.vertices[secondIndex].position);

				const glm::vec3 thirdPosition =
					ToVector3(
						mesh.vertices[thirdIndex].position);

				const glm::vec3 firstEdge =
					secondPosition - firstPosition;

				const glm::vec3 secondEdge =
					thirdPosition - firstPosition;

				const glm::vec3 faceNormal =
					glm::cross(firstEdge, secondEdge);

				const float faceLengthSquared =
					glm::dot(faceNormal, faceNormal);

				if (!std::isfinite(faceLengthSquared))
				{
					throw std::runtime_error(
						"Normal generation encountered "
						"non-finite triangle geometry.");
				}

				if (faceLengthSquared <=
					minimumFaceLengthSquared)
				{
					continue;
				}

				// The unnormalized cross product produces
				// area-weighted smooth normals.
				accumulatedNormals[firstIndex] +=
					faceNormal;

				accumulatedNormals[secondIndex] +=
					faceNormal;

				accumulatedNormals[thirdIndex] +=
					faceNormal;
			}

			std::vector<glm::vec3> generatedNormals;
			generatedNormals.reserve(mesh.vertices.size());

			for (const glm::vec3& accumulatedNormal :
				accumulatedNormals)
			{
				generatedNormals.push_back(
					NormalizeGenerated(
						accumulatedNormal));
			}

			for (std::size_t vertexIndex = 0;
				vertexIndex < mesh.vertices.size();
				++vertexIndex)
			{
				mesh.vertices[vertexIndex].normal =
					ToArray(
						generatedNormals[vertexIndex]);
			}

			mesh.availableAttributes.normals = true;
		}

		void GenerateTangents(MeshData& mesh)
		{
			if (mesh.availableAttributes.tangents)
			{
				return;
			}

			// Without primary texture coordinates, retain the
			// fallback tangent and leave tangents unavailable.
			if (!mesh.availableAttributes.textureCoordinates)
			{
				return;
			}

			if (!mesh.availableAttributes.normals)
			{
				throw std::runtime_error(
					"Tangent generation requires "
					"vertex normals.");
			}

			std::vector<glm::vec3> accumulatedTangents(
				mesh.vertices.size(),
				glm::vec3{ 0.0f });

			std::vector<glm::vec3> accumulatedBitangents(
				mesh.vertices.size(),
				glm::vec3{ 0.0f });

			constexpr float minimumTextureGradient =
				1e-8f;

			for (std::size_t index = 0;
				index < mesh.indices.size();
				index += 3)
			{
				const std::uint32_t firstIndex =
					mesh.indices[index];

				const std::uint32_t secondIndex =
					mesh.indices[index + 1];

				const std::uint32_t thirdIndex =
					mesh.indices[index + 2];

				const MeshVertex& firstVertex =
					mesh.vertices[firstIndex];

				const MeshVertex& secondVertex =
					mesh.vertices[secondIndex];

				const MeshVertex& thirdVertex =
					mesh.vertices[thirdIndex];

				const glm::vec3 firstPosition =
					ToVector3(firstVertex.position);

				const glm::vec3 secondPosition =
					ToVector3(secondVertex.position);

				const glm::vec3 thirdPosition =
					ToVector3(thirdVertex.position);

				const glm::vec3 firstEdge =
					secondPosition - firstPosition;

				const glm::vec3 secondEdge =
					thirdPosition - firstPosition;

				const glm::vec2 firstTextureCoordinate =
					ToVector2(
						firstVertex.textureCoordinates);

				const glm::vec2 secondTextureCoordinate =
					ToVector2(
						secondVertex.textureCoordinates);

				const glm::vec2 thirdTextureCoordinate =
					ToVector2(
						thirdVertex.textureCoordinates);

				const glm::vec2 firstTextureEdge =
					secondTextureCoordinate -
					firstTextureCoordinate;

				const glm::vec2 secondTextureEdge =
					thirdTextureCoordinate -
					firstTextureCoordinate;

				const float determinant =
					firstTextureEdge.x *
					secondTextureEdge.y -
					firstTextureEdge.y *
					secondTextureEdge.x;

				if (!std::isfinite(determinant))
				{
					throw std::runtime_error(
						"Tangent generation encountered "
						"non-finite texture coordinates.");
				}

				if (std::abs(determinant) <=
					minimumTextureGradient)
				{
					continue;
				}

				const float inverseDeterminant =
					1.0f / determinant;

				const glm::vec3 tangent =
					(
						firstEdge *
						secondTextureEdge.y -
						secondEdge *
						firstTextureEdge.y
						) *
					inverseDeterminant;

				const glm::vec3 bitangent =
					(
						secondEdge *
						firstTextureEdge.x -
						firstEdge *
						secondTextureEdge.x
						) *
					inverseDeterminant;

				if (!IsFinite(tangent) ||
					!IsFinite(bitangent))
				{
					throw std::runtime_error(
						"Tangent generation produced "
						"non-finite triangle data.");
				}

				accumulatedTangents[firstIndex] +=
					tangent;

				accumulatedTangents[secondIndex] +=
					tangent;

				accumulatedTangents[thirdIndex] +=
					tangent;

				accumulatedBitangents[firstIndex] +=
					bitangent;

				accumulatedBitangents[secondIndex] +=
					bitangent;

				accumulatedBitangents[thirdIndex] +=
					bitangent;
			}

			std::vector<std::array<float, 4>>
				generatedTangents;

			generatedTangents.reserve(
				mesh.vertices.size());

			for (std::size_t vertexIndex = 0;
				vertexIndex < mesh.vertices.size();
				++vertexIndex)
			{
				const glm::vec3 normal =
					NormalizeGenerated(
						ToVector3(
							mesh.vertices[vertexIndex]
							.normal));

				const glm::vec3& accumulatedTangent =
					accumulatedTangents[vertexIndex];

				const glm::vec3 orthogonalTangent =
					accumulatedTangent -
					normal *
					glm::dot(
						normal,
						accumulatedTangent);

				const float tangentLengthSquared =
					glm::dot(
						orthogonalTangent,
						orthogonalTangent);

				if (!std::isfinite(tangentLengthSquared))
				{
					throw std::runtime_error(
						"Tangent generation produced "
						"non-finite accumulated tangent data.");
				}

				constexpr float minimumBasisLengthSquared = 1e-12f;

				glm::vec3 tangent;
				float handedness = 1.0f;

				if (tangentLengthSquared <= minimumBasisLengthSquared)
				{
					// Choose an axis that is not nearly parallel to the normal.
					// The resulting fallback is deterministic and perpendicular.
					const glm::vec3 referenceAxis =
						std::abs(normal.z) < 0.9f
						? glm::vec3{ 0.0f, 0.0f, 1.0f }
					: glm::vec3{ 0.0f, 1.0f, 0.0f };

					tangent = NormalizeGenerated(
						glm::cross(referenceAxis, normal));
				}
				else
				{
					tangent = NormalizeGenerated(
						orthogonalTangent);
				}

				const glm::vec3& bitangent =
					accumulatedBitangents[vertexIndex];

				const float bitangentLengthSquared =
					glm::dot(bitangent, bitangent);

				if (!std::isfinite(bitangentLengthSquared))
				{
					throw std::runtime_error(
						"Tangent generation produced "
						"non-finite accumulated bitangent data.");
				}

				if (bitangentLengthSquared > minimumBasisLengthSquared)
				{
					const float handednessValue =
						glm::dot(
							glm::cross(normal, tangent),
							bitangent);

					if (!std::isfinite(handednessValue))
					{
						throw std::runtime_error(
							"Tangent generation produced "
							"non-finite handedness.");
					}

					handedness =
						handednessValue < 0.0f
						? -1.0f
						: 1.0f;
				}

				generatedTangents.push_back({
					tangent.x,
					tangent.y,
					tangent.z,
					handedness
					});
			}

			for (std::size_t vertexIndex = 0;
				vertexIndex < mesh.vertices.size();
				++vertexIndex)
			{
				mesh.vertices[vertexIndex].tangent =
					generatedTangents[vertexIndex];
			}

			mesh.availableAttributes.tangents = true;
		}
	}

	void GenerateMissingNormalsAndTangents(
		MeshData& mesh,
		bool generateTangents)
	{
		if (mesh.vertices.empty())
		{
			throw std::invalid_argument(
				"Cannot generate mesh attributes "
				"for an empty mesh.");
		}

		if (mesh.indices.empty() ||
			mesh.indices.size() % 3 != 0)
		{
			throw std::invalid_argument(
				"Mesh attribute generation requires "
				"triangle indices.");
		}

		for (const std::uint32_t index : mesh.indices)
		{
			if (index >= mesh.vertices.size())
			{
				throw std::invalid_argument(
					"Mesh attribute generation encountered "
					"an invalid vertex index.");
			}
		}

		GenerateSmoothNormals(mesh);

		if (generateTangents)
		{
			GenerateTangents(mesh);
		}
	}
}