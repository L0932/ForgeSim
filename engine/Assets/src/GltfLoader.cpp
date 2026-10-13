#include <ForgeSim/Assets/GltfLoader.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <ForgeSim/Assets/ImageLoader.hpp>

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

		using GltfDataPointer =
			std::unique_ptr<
			cgltf_data,
			decltype(&cgltf_free)>;

		[[nodiscard]] GltfDataPointer LoadGltfDocument(
			const std::filesystem::path& path)
		{
			cgltf_options options{};
			cgltf_data* rawData = nullptr;

			const std::string pathString = path.string();

			const cgltf_result parseResult =
				cgltf_parse_file(
					&options,
					pathString.c_str(),
					&rawData);

			GltfDataPointer data{
				rawData,
				&cgltf_free
			};

			if (parseResult != cgltf_result_success)
			{
				ThrowLoadError(
					path,
					"parse",
					parseResult);
			}

			const cgltf_result bufferResult =
				cgltf_load_buffers(
					&options,
					data.get(),
					pathString.c_str());

			if (bufferResult != cgltf_result_success)
			{
				ThrowLoadError(
					path,
					"load buffers for",
					bufferResult);
			}

			const cgltf_result validationResult =
				cgltf_validate(data.get());

			if (validationResult != cgltf_result_success)
			{
				ThrowLoadError(
					path,
					"validate",
					validationResult);
			}

			if (data->animations_count != 0)
			{
				throw std::runtime_error(
					"Animated glTF assets are not currently supported: " +
					path.string());
			}

			if (data->skins_count != 0)
			{
				throw std::runtime_error(
					"Skinned glTF assets are not currently supported: " +
					path.string());
			}

			return data;
		}

		[[nodiscard]] MeshData LoadPrimitiveGeometry(
			const cgltf_primitive& primitive,
			const std::filesystem::path& path)
		{
			if (primitive.targets_count != 0)
			{
				throw std::runtime_error(
					"glTF morph targets are not currently supported: " +
					path.string());
			}

			if (primitive.type != cgltf_primitive_type_triangles)
			{
				throw std::runtime_error(
					"glTF primitive is not composed of triangles: " +
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
					"glTF primitive has no POSITION attribute: " +
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

		template<typename Element>
		[[nodiscard]] std::size_t FindReferencedIndex(
			const Element* reference,
			const Element* elements,
			cgltf_size elementCount,
			std::string_view referenceDescription,
			const std::filesystem::path& path)
		{
			for (cgltf_size index = 0;
				index < elementCount;
				++index)
			{
				if (reference == &elements[index])
				{
					return static_cast<std::size_t>(index);
				}
			}

			throw std::runtime_error(
				"glTF " +
				std::string{ referenceDescription } +
				" contains an invalid reference: " +
				path.string());
		}

		[[nodiscard]] MaterialTextureReference
			LoadMaterialTextureReference(
				const cgltf_texture_view& textureView,
				const cgltf_data& data,
				const std::filesystem::path& path)
		{
			if (textureView.texture == nullptr)
			{
				throw std::runtime_error(
					"glTF material contains an empty texture reference: " +
					path.string());
			}

			if (textureView.texcoord < 0)
			{
				throw std::runtime_error(
					"glTF material contains a negative texture-coordinate set: " +
					path.string());
			}

			return MaterialTextureReference{
				.textureIndex =
					FindReferencedIndex(
						textureView.texture,
						data.textures,
						data.textures_count,
						"material texture",
						path),

				.textureCoordinateSet =
					static_cast<std::uint32_t>(
						textureView.texcoord)
			};
		}

		[[nodiscard]] AlphaMode ConvertAlphaMode(
			cgltf_alpha_mode alphaMode)
		{
			switch (alphaMode)
			{
			case cgltf_alpha_mode_opaque:
				return AlphaMode::Opaque;

			case cgltf_alpha_mode_mask:
				return AlphaMode::Mask;

			case cgltf_alpha_mode_blend:
				return AlphaMode::Blend;

			default:
				throw std::runtime_error(
					"glTF material contains an unsupported alpha mode.");
			}
		}

		[[nodiscard]] MaterialData LoadMaterial(
			const cgltf_material& sourceMaterial,
			const cgltf_data& data,
			const std::filesystem::path& path)
		{
			MaterialData material;

			if (sourceMaterial.has_pbr_metallic_roughness)
			{
				const cgltf_pbr_metallic_roughness&
					sourceMetallicRoughness =
					sourceMaterial.pbr_metallic_roughness;

				for (std::size_t component = 0;
					component < 4;
					++component)
				{
					material.metallicRoughness
						.baseColorFactor[component] =
						sourceMetallicRoughness
						.base_color_factor[component];
				}

				material.metallicRoughness.metallicFactor =
					sourceMetallicRoughness.metallic_factor;

				material.metallicRoughness.roughnessFactor =
					sourceMetallicRoughness.roughness_factor;

				if (sourceMetallicRoughness
					.base_color_texture.texture != nullptr)
				{
					material.metallicRoughness.baseColorTexture =
						LoadMaterialTextureReference(
							sourceMetallicRoughness
							.base_color_texture,
							data,
							path);
				}

				if (sourceMetallicRoughness
					.metallic_roughness_texture.texture != nullptr)
				{
					material.metallicRoughness
						.metallicRoughnessTexture =
						LoadMaterialTextureReference(
							sourceMetallicRoughness
							.metallic_roughness_texture,
							data,
							path);
				}
			}

			if (sourceMaterial.normal_texture.texture != nullptr)
			{
				material.normalTexture =
					NormalTextureReference{
						.texture =
							LoadMaterialTextureReference(
								sourceMaterial.normal_texture,
								data,
								path),
						.scale =
							sourceMaterial.normal_texture.scale
				};
			}

			if (sourceMaterial.occlusion_texture.texture != nullptr)
			{
				material.occlusionTexture =
					OcclusionTextureReference{
						.texture =
							LoadMaterialTextureReference(
								sourceMaterial.occlusion_texture,
								data,
								path),
						.strength =
							sourceMaterial.occlusion_texture.scale
				};
			}

			for (std::size_t component = 0;
				component < 3;
				++component)
			{
				material.emissiveFactor[component] =
					sourceMaterial.emissive_factor[component];
			}

			if (sourceMaterial.emissive_texture.texture != nullptr)
			{
				material.emissiveTexture =
					LoadMaterialTextureReference(
						sourceMaterial.emissive_texture,
						data,
						path);
			}

			material.alphaMode =
				ConvertAlphaMode(sourceMaterial.alpha_mode);

			material.alphaCutoff =
				sourceMaterial.alpha_cutoff;

			material.doubleSided =
				sourceMaterial.double_sided;

			return material;
		}

		[[nodiscard]] std::optional<TextureFilter>
			ConvertTextureFilter(
				cgltf_filter_type filter)
		{
			switch (filter)
			{
			case cgltf_filter_type_undefined:
				return std::nullopt;

			case cgltf_filter_type_nearest:
				return TextureFilter::Nearest;

			case cgltf_filter_type_linear:
				return TextureFilter::Linear;

			case cgltf_filter_type_nearest_mipmap_nearest:
				return TextureFilter::NearestMipmapNearest;

			case cgltf_filter_type_linear_mipmap_nearest:
				return TextureFilter::LinearMipmapNearest;

			case cgltf_filter_type_nearest_mipmap_linear:
				return TextureFilter::NearestMipmapLinear;

			case cgltf_filter_type_linear_mipmap_linear:
				return TextureFilter::LinearMipmapLinear;
			}

			throw std::runtime_error(
				"glTF sampler contains an unsupported texture filter.");
		}

		[[nodiscard]] TextureWrap ConvertTextureWrap(
			cgltf_wrap_mode wrapMode)
		{
			switch (wrapMode)
			{
			case cgltf_wrap_mode_repeat:
				return TextureWrap::Repeat;

			case cgltf_wrap_mode_mirrored_repeat:
				return TextureWrap::MirroredRepeat;

			case cgltf_wrap_mode_clamp_to_edge:
				return TextureWrap::ClampToEdge;
			}

			throw std::runtime_error(
				"glTF sampler contains an unsupported texture wrap mode.");
		}

		[[nodiscard]] SamplerData LoadSampler(
			const cgltf_sampler& sourceSampler)
		{
			return SamplerData{
				.minificationFilter =
					ConvertTextureFilter(
						sourceSampler.min_filter),

				.magnificationFilter =
					ConvertTextureFilter(
						sourceSampler.mag_filter),

				.wrapU =
					ConvertTextureWrap(
						sourceSampler.wrap_s),

				.wrapV =
					ConvertTextureWrap(
						sourceSampler.wrap_t)
			};
		}

		[[nodiscard]] std::size_t Base64DecodedSize(
			std::string_view encodedData,
			const std::filesystem::path& path)
		{
			if (encodedData.empty() ||
				encodedData.size() % 4 != 0)
			{
				throw std::runtime_error(
					"glTF image contains malformed base64 data: " +
					path.string());
			}

			std::size_t decodedSize =
				encodedData.size() / 4 * 3;

			if (encodedData.ends_with("=="))
			{
				decodedSize -= 2;
			}
			else if (encodedData.ends_with("="))
			{
				decodedSize -= 1;
			}

			return decodedSize;
		}

		[[nodiscard]] ImageData LoadUriImage(
			const cgltf_image& sourceImage,
			const std::filesystem::path& gltfPath,
			std::string_view sourceName)
		{
			if (sourceImage.uri == nullptr)
			{
				throw std::runtime_error(
					"glTF URI image has no URI: " +
					gltfPath.string());
			}

			const std::string_view uri{
				sourceImage.uri
			};

			if (!uri.starts_with("data:"))
			{
				if (uri.find("://") !=
					std::string_view::npos)
				{
					throw std::runtime_error(
						"Remote glTF image URIs are not supported: " +
						std::string{ uri });
				}

				std::string decodedUri{
					uri
				};

				const cgltf_size decodedLength =
					cgltf_decode_uri(
						decodedUri.data());

				decodedUri.resize(
					static_cast<std::size_t>(
						decodedLength));

				return LoadImage(
					gltfPath.parent_path() /
					std::filesystem::path{
						decodedUri
					});
			}

			const std::size_t commaPosition =
				uri.find(',');

			if (commaPosition ==
				std::string_view::npos)
			{
				throw std::runtime_error(
					"glTF image contains a malformed data URI: " +
					gltfPath.string());
			}

			const std::string_view metadata =
				uri.substr(
					0,
					commaPosition);

			if (!metadata.ends_with(";base64"))
			{
				throw std::runtime_error(
					"Only base64 glTF image data URIs are supported: " +
					gltfPath.string());
			}

			const std::string_view base64Data =
				uri.substr(
					commaPosition + 1);

			const std::size_t decodedSize =
				Base64DecodedSize(
					base64Data,
					gltfPath);

			cgltf_options options{};

			options.memory.alloc_func =
				[](void*, cgltf_size size) -> void*
				{
					return std::malloc(size);
				};

			options.memory.free_func =
				[](void*, void* memory)
				{
					std::free(memory);
				};

			void* rawDecodedData = nullptr;

			const cgltf_result result =
				cgltf_load_buffer_base64(
					&options,
					decodedSize,
					base64Data.data(),
					&rawDecodedData);

			using DecodedDataPointer =
				std::unique_ptr<
				void,
				decltype(&std::free)>;

			DecodedDataPointer decodedData{
				rawDecodedData,
				&std::free
			};

			if (result != cgltf_result_success)
			{
				ThrowLoadError(
					gltfPath,
					"decode an embedded image from",
					result);
			}

			const auto* bytes =
				static_cast<const std::uint8_t*>(
					decodedData.get());

			return DecodeImage(
				std::span<const std::uint8_t>{
				bytes,
					decodedSize
			},
				sourceName);
		}

		[[nodiscard]] ImageData LoadGltfImage(
			const cgltf_image& sourceImage,
			const std::filesystem::path& gltfPath,
			std::size_t imageIndex)
		{
			const std::string sourceName =
				sourceImage.name != nullptr
				? std::string{ sourceImage.name }
				: gltfPath.string() +
				" image " +
				std::to_string(imageIndex);

			if (sourceImage.buffer_view != nullptr)
			{
				const std::uint8_t* encodedData =
					cgltf_buffer_view_data(
						sourceImage.buffer_view);

				const std::size_t encodedSize =
					static_cast<std::size_t>(
						sourceImage.buffer_view->size);

				if (encodedData == nullptr ||
					encodedSize == 0)
				{
					throw std::runtime_error(
						"glTF image buffer view contains no data: " +
						sourceName);
				}

				return DecodeImage(
					std::span<const std::uint8_t>{
					encodedData,
						encodedSize
				},
					sourceName);
			}

			if (sourceImage.uri != nullptr)
			{
				return LoadUriImage(
					sourceImage,
					gltfPath,
					sourceName);
			}

			throw std::runtime_error(
				"glTF image has neither a URI nor a buffer view: " +
				sourceName);
		}
	} // EOF anonymous namespace

	MeshData LoadGltfMesh(const std::filesystem::path& path)
	{
		const GltfDataPointer data =
			LoadGltfDocument(path);

		if (data->meshes_count == 0)
		{
			throw std::runtime_error(
				"glTF file contains no meshes: " +
				path.string());
		}

		if (data->meshes_count != 1)
		{
			throw std::runtime_error(
				"Expected exactly one mesh in glTF file: " +
				path.string());
		}

		const cgltf_mesh& sourceMesh =
			data->meshes[0];

		if (sourceMesh.primitives_count != 1)
		{
			throw std::runtime_error(
				"Expected exactly one primitive in glTF mesh: " +
				path.string());
		}

		return LoadPrimitiveGeometry(
			sourceMesh.primitives[0],
			path);
	}

	SceneAsset LoadGltfScene(
		const std::filesystem::path& path)
	{
		const GltfDataPointer data =
			LoadGltfDocument(path);

		SceneAsset sceneAsset;

		if (data->scene != nullptr &&
			data->scene->name != nullptr)
		{
			sceneAsset.name =
				data->scene->name;
		}

		sceneAsset.materials.reserve(
			data->materials_count);

		for (cgltf_size materialIndex = 0;
			materialIndex < data->materials_count;
			++materialIndex)
		{
			sceneAsset.materials.push_back(
				LoadMaterial(
					data->materials[materialIndex],
					*data,
					path));
		}

		sceneAsset.textureResources.images.reserve(
			data->images_count);

		for (cgltf_size imageIndex = 0;
			imageIndex < data->images_count;
			++imageIndex)
		{
			sceneAsset.textureResources.images.push_back(
				LoadGltfImage(
					data->images[imageIndex],
					path,
					static_cast<std::size_t>(
						imageIndex)));
		}

		sceneAsset.textureResources.samplers.reserve(
			data->samplers_count);

		for (cgltf_size samplerIndex = 0;
			samplerIndex < data->samplers_count;
			++samplerIndex)
		{
			sceneAsset.textureResources.samplers.push_back(
				LoadSampler(
					data->samplers[samplerIndex]));
		}

		sceneAsset.textureResources.textures.reserve(
			data->textures_count);

		for (cgltf_size textureIndex = 0;
			textureIndex < data->textures_count;
			++textureIndex)
		{
			const cgltf_texture& sourceTexture =
				data->textures[textureIndex];

			if (sourceTexture.image == nullptr)
			{
				throw std::runtime_error(
					"glTF texture has no image source: " +
					path.string());
			}

			TextureData texture{
				.imageIndex =
					FindReferencedIndex(
						sourceTexture.image,
						data->images,
						data->images_count,
						"texture image",
						path),
				.samplerIndex = std::nullopt
			};

			if (sourceTexture.sampler != nullptr)
			{
				texture.samplerIndex =
					FindReferencedIndex(
						sourceTexture.sampler,
						data->samplers,
						data->samplers_count,
						"texture sampler",
						path);
			}

			sceneAsset.textureResources.textures.push_back(
				std::move(texture));
		}

		sceneAsset.meshes.reserve(
			data->meshes_count);

		for (cgltf_size meshIndex = 0;
			meshIndex < data->meshes_count;
			++meshIndex)
		{
			const cgltf_mesh& sourceMesh =
				data->meshes[meshIndex];

			SceneMeshData destinationMesh;

			if (sourceMesh.name != nullptr)
			{
				destinationMesh.name =
					sourceMesh.name;
			}

			destinationMesh.primitives.reserve(
				sourceMesh.primitives_count);

			for (cgltf_size primitiveIndex = 0;
				primitiveIndex <
				sourceMesh.primitives_count;
				++primitiveIndex)
			{
				const cgltf_primitive& sourcePrimitive =
					sourceMesh.primitives[primitiveIndex];

				MeshPrimitiveData destinationPrimitive{
					.geometry =
						LoadPrimitiveGeometry(
							sourcePrimitive,
							path),
					.materialIndex =
						sourcePrimitive.material != nullptr
						? std::optional<MaterialIndex>{
							FindReferencedIndex(
								sourcePrimitive.material,
								data->materials,
								data->materials_count,
								"primitive material",
								path)
						}
						: std::nullopt
				};

				destinationMesh.primitives.push_back(
					std::move(destinationPrimitive));
			}

			sceneAsset.meshes.push_back(
				std::move(destinationMesh));
		}

		sceneAsset.nodes.reserve(
			data->nodes_count);

		for (cgltf_size nodeIndex = 0;
			nodeIndex < data->nodes_count;
			++nodeIndex)
		{
			const cgltf_node& sourceNode =
				data->nodes[nodeIndex];

			SceneNodeData destinationNode;

			if (sourceNode.name != nullptr)
			{
				destinationNode.name =
					sourceNode.name;
			}

			if (sourceNode.has_translation)
			{
				for (std::size_t component = 0;
					component < 3;
					++component)
				{
					destinationNode.localTransform
						.translation[component] =
						sourceNode.translation[component];
				}
			}

			if (sourceNode.has_rotation)
			{
				for (std::size_t component = 0;
					component < 4;
					++component)
				{
					destinationNode.localTransform
						.rotation[component] =
						sourceNode.rotation[component];
				}
			}

			if (sourceNode.has_scale)
			{
				for (std::size_t component = 0;
					component < 3;
					++component)
				{
					destinationNode.localTransform
						.scale[component] =
						sourceNode.scale[component];
				}
			}

			if (sourceNode.has_matrix)
			{
				std::array<float, 16> matrix{};

				for (std::size_t component = 0;
					component < matrix.size();
					++component)
				{
					matrix[component] =
						sourceNode.matrix[component];
				}

				destinationNode.localTransform.matrix =
					matrix;
			}

			if (sourceNode.mesh != nullptr)
			{
				destinationNode.meshIndex =
					FindReferencedIndex(
						sourceNode.mesh,
						data->meshes,
						data->meshes_count,
						"node mesh",
						path);
			}

			destinationNode.childIndices.reserve(
				sourceNode.children_count);

			for (cgltf_size childIndex = 0;
				childIndex < sourceNode.children_count;
				++childIndex)
			{
				destinationNode.childIndices.push_back(
					FindReferencedIndex(
						sourceNode.children[childIndex],
						data->nodes,
						data->nodes_count,
						"node child",
						path));
			}

			sceneAsset.nodes.push_back(
				std::move(destinationNode));
		}

		if (data->scene != nullptr)
		{
			sceneAsset.rootNodeIndices.reserve(
				data->scene->nodes_count);

			for (cgltf_size rootIndex = 0;
				rootIndex < data->scene->nodes_count;
				++rootIndex)
			{
				sceneAsset.rootNodeIndices.push_back(
					FindReferencedIndex(
						data->scene->nodes[rootIndex],
						data->nodes,
						data->nodes_count,
						"scene root node",
						path));
			}
		}
		else
		{
			// A default scene is optional in glTF. Treat every
			// parentless node as a root when none is designated.
			for (cgltf_size nodeIndex = 0;
				nodeIndex < data->nodes_count;
				++nodeIndex)
			{
				if (data->nodes[nodeIndex].parent == nullptr)
				{
					sceneAsset.rootNodeIndices.push_back(
						static_cast<SceneNodeIndex>(
							nodeIndex));
				}
			}
		}

		if (!sceneAsset.textureResources.HasValidReferences())
		{
			throw std::runtime_error(
				"Loaded glTF texture resources contain "
				"invalid image or sampler references: " +
				path.string());
		}

		return sceneAsset;
	}
}