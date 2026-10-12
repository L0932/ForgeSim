#include <ForgeSim/Assets/GltfLoader.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

namespace ForgeSim::Assets
{
	namespace
	{
		[[nodiscard]] bool UsesFloatComponents(
			const cgltf_accessor& accessor) noexcept
		{
			return accessor.component_type ==
				cgltf_component_type_r_32f;
		}

		[[nodiscard]] bool UsesSupportedNormalizedComponents(
			const cgltf_accessor& accessor) noexcept
		{
			switch (accessor.component_type)
			{
			case cgltf_component_type_r_32f:
				return !accessor.normalized;

			case cgltf_component_type_r_8u:
			case cgltf_component_type_r_16u:
				return accessor.normalized;

			default:
				return false;
			}
		}

		[[nodiscard]] std::string_view CgltfResultDescription(
			cgltf_result result) noexcept
		{
			switch (result)
			{
			case cgltf_result_success:
				return "success";

			case cgltf_result_data_too_short:
				return "data too short";

			case cgltf_result_unknown_format:
				return "unknown format";

			case cgltf_result_invalid_json:
				return "invalid JSON";

			case cgltf_result_invalid_gltf:
				return "invalid glTF";

			case cgltf_result_invalid_options:
				return "invalid options";

			case cgltf_result_file_not_found:
				return "file not found";

			case cgltf_result_io_error:
				return "I/O error";

			case cgltf_result_out_of_memory:
				return "out of memory";

			case cgltf_result_legacy_gltf:
				return "legacy glTF is unsupported";
			}

			return "unknown error";
		}

		[[noreturn]] void ThrowLoadError(
			const std::filesystem::path& path,
			std::string_view operation,
			cgltf_result result)
		{
			throw std::runtime_error(
				"Failed to " +
				std::string(operation) +
				" glTF file '" +
				path.string() +
				"': " +
				std::string(CgltfResultDescription(result)));
		}
	}

	MeshData LoadGltfMesh(const std::filesystem::path& path)
	{
		const std::string pathString = path.string();

		cgltf_options options{};
		cgltf_data* rawData = nullptr;

		const cgltf_result parseResult =
			cgltf_parse_file(
				&options,
				pathString.c_str(),
				&rawData);

		std::unique_ptr<cgltf_data, decltype(&cgltf_free)> data{
			rawData,
			&cgltf_free
		};

		if (parseResult != cgltf_result_success)
		{
			ThrowLoadError(path, "parse", parseResult);
		}

		const cgltf_result bufferResult =
			cgltf_load_buffers(
				&options,
				data.get(),
				pathString.c_str());

		if (bufferResult != cgltf_result_success)
		{
			ThrowLoadError(path, "load buffers for", bufferResult);
		}

		const cgltf_result validationResult =
			cgltf_validate(data.get());

		if (validationResult != cgltf_result_success)
		{
			ThrowLoadError(path, "validate", validationResult);
		}

		if (data->animations_count != 0)
		{
			throw std::runtime_error(
				"Animated glTF models are not supported: " +
				path.string());
		}

		if (data->skins_count != 0)
		{
			throw std::runtime_error(
				"Skinned glTF models are not supported: " +
				path.string());
		}

		if (data->meshes_count == 0)
		{
			throw std::runtime_error(
				"glTF file contains no meshes: " +
				path.string());
		}

		if (data->meshes_count != 1)
		{
			throw std::runtime_error(
				"glTF files containing multiple meshes "
				"are not currently supported: " +
				path.string());
		}

		const cgltf_mesh& mesh = data->meshes[0];

		if (mesh.primitives_count == 0)
		{
			throw std::runtime_error(
				"First glTF mesh contains no primitives: " +
				path.string());
		}

		if (mesh.primitives_count != 1)
		{
			throw std::runtime_error(
				"glTF meshes containing multiple primitives "
				"are not currently supported: " +
				path.string());
		}

		const cgltf_primitive& primitive =
			mesh.primitives[0];

		if (primitive.targets_count != 0)
		{
			throw std::runtime_error(
				"glTF morph targets are not currently supported: " +
				path.string());
		}

		if (primitive.type != cgltf_primitive_type_triangles)
		{
			throw std::runtime_error(
				"First glTF primitive is not composed of triangles: " +
				path.string());
		}

		const cgltf_accessor* positionAccessor = nullptr;
		const cgltf_accessor* normalAccessor = nullptr;
		const cgltf_accessor* textureCoordinateAccessor = nullptr;
		const cgltf_accessor* tangentAccessor = nullptr;
		const cgltf_accessor* colorAccessor = nullptr;

		for (cgltf_size attributeIndex = 0;
			attributeIndex < primitive.attributes_count;
			++attributeIndex)
		{
			const cgltf_attribute& attribute =
				primitive.attributes[attributeIndex];

			switch (attribute.type)
			{
			case cgltf_attribute_type_position:
				positionAccessor = attribute.data;
				break;

			case cgltf_attribute_type_normal:
				normalAccessor = attribute.data;
				break;

			case cgltf_attribute_type_texcoord:
				if (attribute.index == 0)
				{
					textureCoordinateAccessor = attribute.data;
				}
				break;

			case cgltf_attribute_type_tangent:
				tangentAccessor = attribute.data;
				break;

			case cgltf_attribute_type_color:
				if (attribute.index == 0)
				{
					colorAccessor = attribute.data;
				}
				break;

			default:
				break;
			}
		}

		if (positionAccessor == nullptr)
		{
			throw std::runtime_error(
				"First glTF primitive has no POSITION attribute: " +
				path.string());
		}

		if (positionAccessor->type != cgltf_type_vec3)
		{
			throw std::runtime_error(
				"glTF POSITION attribute is not a three-component vector: " +
				path.string());
		}

		if (positionAccessor->count >
			std::numeric_limits<std::uint32_t>::max())
		{
			throw std::runtime_error(
				"glTF mesh contains too many vertices: " +
				path.string());
		}

		const cgltf_size vertexCount =
			positionAccessor->count;

		const auto validateOptionalAttribute =
			[vertexCount](
				const cgltf_accessor* accessor,
				cgltf_type expectedType,
				std::string_view attributeName)
			{
				if (accessor == nullptr)
				{
					return;
				}

				if (accessor->count != vertexCount)
				{
					throw std::runtime_error(
						std::string(attributeName) +
						" vertex count does not match POSITION.");
				}

				if (accessor->type != expectedType)
				{
					throw std::runtime_error(
						std::string(attributeName) +
						" has an unsupported element type.");
				}
			};

		validateOptionalAttribute(
			normalAccessor,
			cgltf_type_vec3,
			"NORMAL");

		validateOptionalAttribute(
			textureCoordinateAccessor,
			cgltf_type_vec2,
			"TEXCOORD_0");

		validateOptionalAttribute(
			tangentAccessor,
			cgltf_type_vec4,
			"TANGENT");

		if (colorAccessor != nullptr)
		{
			if (colorAccessor->count != positionAccessor->count)
			{
				throw std::runtime_error(
					"glTF COLOR_0 and POSITION counts do not match: " +
					path.string());
			}

			if (colorAccessor->type != cgltf_type_vec3 &&
				colorAccessor->type != cgltf_type_vec4)
			{
				throw std::runtime_error(
					"glTF COLOR_0 attribute must contain RGB or RGBA values: " +
					path.string());
			}
		}

		if (!UsesFloatComponents(*positionAccessor))
		{
			throw std::runtime_error(
				"POSITION must use floating-point components.");
		}

		if (normalAccessor != nullptr &&
			!UsesFloatComponents(*normalAccessor))
		{
			throw std::runtime_error(
				"NORMAL must use floating-point components.");
		}

		if (tangentAccessor != nullptr &&
			!UsesFloatComponents(*tangentAccessor))
		{
			throw std::runtime_error(
				"TANGENT must use floating-point components.");
		}

		if (textureCoordinateAccessor != nullptr &&
			!UsesSupportedNormalizedComponents(
				*textureCoordinateAccessor))
		{
			throw std::runtime_error(
				"TEXCOORD_0 must use floating-point or "
				"normalized unsigned integer components.");
		}

		if (colorAccessor != nullptr &&
			!UsesSupportedNormalizedComponents(*colorAccessor))
		{
			throw std::runtime_error(
				"COLOR_0 must use floating-point or "
				"normalized unsigned integer components.");
		}

		MeshData meshData;

		meshData.availableAttributes = {
			.normals = normalAccessor != nullptr,
			.textureCoordinates =
				textureCoordinateAccessor != nullptr,
			.tangents = tangentAccessor != nullptr,
			.colors = colorAccessor != nullptr
		};

		meshData.vertices.reserve(
			static_cast<std::size_t>(vertexCount));

		for (cgltf_size vertexIndex = 0;
			vertexIndex < vertexCount;
			++vertexIndex)
		{
			std::array<float, 3> position{};

			std::array<float, 3> normal{
				0.0f,
				0.0f,
				1.0f
			};

			std::array<float, 2> textureCoordinates{};

			std::array<float, 4> tangent{
				1.0f,
				0.0f,
				0.0f,
				1.0f
			};

			std::array<float, 4> color{
				1.0f,
				1.0f,
				1.0f,
				1.0f
			};

			if (!cgltf_accessor_read_float(
				positionAccessor,
				vertexIndex,
				position.data(),
				position.size()))
			{
				throw std::runtime_error(
					"Failed to read POSITION vertex attribute.");
			}

			if (normalAccessor != nullptr &&
				!cgltf_accessor_read_float(
					normalAccessor,
					vertexIndex,
					normal.data(),
					normal.size()))
			{
				throw std::runtime_error(
					"Failed to read NORMAL vertex attribute.");
			}

			if (textureCoordinateAccessor != nullptr &&
				!cgltf_accessor_read_float(
					textureCoordinateAccessor,
					vertexIndex,
					textureCoordinates.data(),
					textureCoordinates.size()))
			{
				throw std::runtime_error(
					"Failed to read TEXCOORD_0 vertex attribute.");
			}

			if (tangentAccessor != nullptr &&
				!cgltf_accessor_read_float(
					tangentAccessor,
					vertexIndex,
					tangent.data(),
					tangent.size()))
			{
				throw std::runtime_error(
					"Failed to read TANGENT vertex attribute.");
			}

			if (colorAccessor != nullptr &&
				!cgltf_accessor_read_float(
					colorAccessor,
					vertexIndex,
					color.data(),
					color.size()))
			{
				throw std::runtime_error(
					"Failed to read COLOR_0 vertex attribute.");
			}

			meshData.vertices.push_back(
				MeshVertex{
					.position = position,
					.normal = normal,
					.textureCoordinates =
						textureCoordinates,
					.tangent = tangent,
					.color = {
						color[0],
						color[1],
						color[2]
					}
				});
		}

		if (primitive.indices != nullptr)
		{
			const cgltf_accessor& indexAccessor =
				*primitive.indices;

			if (indexAccessor.type != cgltf_type_scalar)
			{
				throw std::runtime_error(
					"glTF index accessor is not scalar: " +
					path.string());
			}

			meshData.indices.reserve(indexAccessor.count);

			for (cgltf_size index = 0;
				index < indexAccessor.count;
				++index)
			{
				const cgltf_size vertexIndex =
					cgltf_accessor_read_index(
						&indexAccessor,
						index);

				if (vertexIndex >= meshData.vertices.size())
				{
					throw std::runtime_error(
						"glTF index references a nonexistent vertex: " +
						path.string());
				}

				meshData.indices.push_back(
					static_cast<std::uint32_t>(vertexIndex));
			}
		}
		else
		{
			meshData.indices.reserve(meshData.vertices.size());

			for (std::uint32_t vertexIndex = 0;
				vertexIndex < meshData.vertices.size();
				++vertexIndex)
			{
				meshData.indices.push_back(vertexIndex);
			}
		}

		if (meshData.indices.size() % 3 != 0)
		{
			throw std::runtime_error(
				"glTF triangle index count is not divisible by three: " +
				path.string());
		}

		return meshData;
	}
}