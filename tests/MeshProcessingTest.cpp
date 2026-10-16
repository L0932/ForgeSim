#include <ForgeSim/Assets/MeshProcessing.hpp>

#include <cmath>
#include <cstddef>
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

    try
    {
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

            GenerateMissingNormalsAndTangents(mesh, true);

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

                const float lengthSquared =
                    vertex.normal[0] * vertex.normal[0] +
                    vertex.normal[1] * vertex.normal[1] +
                    vertex.normal[2] * vertex.normal[2];

                if (!NearlyEqual(lengthSquared, 1.0f))
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

            GenerateMissingNormalsAndTangents(mesh, true);

            for (const MeshVertex& vertex : mesh.vertices)
            {
                if (!NearlyEqual(vertex.normal[0], 0.0f) ||
                    !NearlyEqual(vertex.normal[1], 1.0f) ||
                    !NearlyEqual(vertex.normal[2], 0.0f))
                {
                    std::cerr << "Authored normal was modified\n";
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
                GenerateMissingNormalsAndTangents(mesh, true);
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

            mesh.availableAttributes.textureCoordinates = true;

            GenerateMissingNormalsAndTangents(mesh, true);

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
                        << "Generated tangent had an unexpected direction\n";

                    return EXIT_FAILURE;
                }

                const float lengthSquared =
                    vertex.tangent[0] * vertex.tangent[0] +
                    vertex.tangent[1] * vertex.tangent[1] +
                    vertex.tangent[2] * vertex.tangent[2];

                const float normalDotTangent =
                    vertex.normal[0] * vertex.tangent[0] +
                    vertex.normal[1] * vertex.tangent[1] +
                    vertex.normal[2] * vertex.tangent[2];

                if (!NearlyEqual(lengthSquared, 1.0f) ||
                    !NearlyEqual(normalDotTangent, 0.0f) ||
                    !NearlyEqual(vertex.tangent[3], 1.0f))
                {
                    std::cerr
                        << "Generated tangent basis was incorrect\n";

                    return EXIT_FAILURE;
                }
            }
        }

        // Authored normals and tangents must remain unchanged,
        // whether tangent generation is enabled or disabled.
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

            const MeshData originalMesh = mesh;

            for (const bool generateTangents : { false, true })
            {
                GenerateMissingNormalsAndTangents(
                    mesh,
                    generateTangents);

                if (!mesh.availableAttributes.normals ||
                    !mesh.availableAttributes.tangents)
                {
                    std::cerr
                        << "Authored attribute availability was modified\n";

                    return EXIT_FAILURE;
                }

                for (std::size_t vertexIndex = 0;
                    vertexIndex < mesh.vertices.size();
                    ++vertexIndex)
                {
                    if (mesh.vertices[vertexIndex].normal !=
                        originalMesh.vertices[vertexIndex].normal ||
                        mesh.vertices[vertexIndex].tangent !=
                        originalMesh.vertices[vertexIndex].tangent)
                    {
                        std::cerr
                            << "Authored normal or tangent was modified\n";

                        return EXIT_FAILURE;
                    }
                }
            }
        }

        // Meshes without texture coordinates should retain their
        // fallback tangents and remain marked as lacking tangents.
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

            const MeshData originalMesh = mesh;

            GenerateMissingNormalsAndTangents(mesh, true);

            if (mesh.availableAttributes.tangents)
            {
                std::cerr
                    << "Tangents were reported without texture coordinates\n";

                return EXIT_FAILURE;
            }

            for (std::size_t vertexIndex = 0;
                vertexIndex < mesh.vertices.size();
                ++vertexIndex)
            {
                if (mesh.vertices[vertexIndex].tangent !=
                    originalMesh.vertices[vertexIndex].tangent)
                {
                    std::cerr
                        << "Fallback tangent was unexpectedly modified\n";

                    return EXIT_FAILURE;
                }
            }
        }

        // Degenerate UVs should receive finite, normalized fallback
        // tangents perpendicular to the authored normals.
        // Exercise both reference-axis branches.
        {
            for (const bool normalAlongZ : { false, true })
            {
                MeshData mesh;

                mesh.vertices = {
                    MeshVertex{
                        .position = { 0.0f, 0.0f, 0.0f },
                        .textureCoordinates = { 0.0f, 0.0f }
                    },
                    MeshVertex{
                        .position = { 1.0f, 0.0f, 0.0f },
                        .textureCoordinates = { 0.0f, 0.0f }
                    },
                    MeshVertex{
                        .position = { 0.0f, 1.0f, 0.0f },
                        .textureCoordinates = { 0.0f, 0.0f }
                    }
                };

                for (MeshVertex& vertex : mesh.vertices)
                {
                    if (normalAlongZ)
                    {
                        vertex.normal = { 0.0f, 0.0f, 1.0f };
                    }
                    else
                    {
                        vertex.normal = { 0.0f, 1.0f, 0.0f };
                    }
                }

                mesh.indices = { 0, 1, 2 };
                mesh.availableAttributes.normals = true;
                mesh.availableAttributes.textureCoordinates = true;

                const MeshData originalMesh = mesh;

                GenerateMissingNormalsAndTangents(mesh, true);

                if (!mesh.availableAttributes.tangents)
                {
                    std::cerr
                        << "Expected fallback tangents to be available\n";

                    return EXIT_FAILURE;
                }

                for (std::size_t vertexIndex = 0;
                    vertexIndex < mesh.vertices.size();
                    ++vertexIndex)
                {
                    const MeshVertex& vertex =
                        mesh.vertices[vertexIndex];

                    if (vertex.normal !=
                        originalMesh.vertices[vertexIndex].normal)
                    {
                        std::cerr
                            << "Fallback generation modified an authored normal\n";

                        return EXIT_FAILURE;
                    }

                    for (const float component : vertex.tangent)
                    {
                        if (!std::isfinite(component))
                        {
                            std::cerr
                                << "Fallback tangent was not finite\n";

                            return EXIT_FAILURE;
                        }
                    }

                    const float lengthSquared =
                        vertex.tangent[0] * vertex.tangent[0] +
                        vertex.tangent[1] * vertex.tangent[1] +
                        vertex.tangent[2] * vertex.tangent[2];

                    const float normalDotTangent =
                        vertex.normal[0] * vertex.tangent[0] +
                        vertex.normal[1] * vertex.tangent[1] +
                        vertex.normal[2] * vertex.tangent[2];

                    if (!NearlyEqual(lengthSquared, 1.0f) ||
                        !NearlyEqual(normalDotTangent, 0.0f) ||
                        !NearlyEqual(vertex.tangent[3], 1.0f))
                    {
                        std::cerr << "Invalid fallback tangent basis\n";
                        return EXIT_FAILURE;
                    }
                }
            }
        }

        // Mirrored texture coordinates should generate negative
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

            GenerateMissingNormalsAndTangents(mesh, true);

            if (!mesh.availableAttributes.tangents)
            {
                std::cerr << "Mirrored UVs did not generate tangents\n";
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
                        << "Mirrored UV tangent had incorrect "
                        << "direction or handedness\n";

                    return EXIT_FAILURE;
                }
            }
        }

        // Degenerate UVs must not prevent normal generation
        // when tangent generation is disabled.
        {
            MeshData mesh;

            mesh.vertices = {
                MeshVertex{
                    .position = { 0.0f, 0.0f, 0.0f },
                    .textureCoordinates = { 0.0f, 0.0f }
                },
                MeshVertex{
                    .position = { 1.0f, 0.0f, 0.0f },
                    .textureCoordinates = { 0.0f, 0.0f }
                },
                MeshVertex{
                    .position = { 0.0f, 1.0f, 0.0f },
                    .textureCoordinates = { 0.0f, 0.0f }
                }
            };

            mesh.indices = { 0, 1, 2 };
            mesh.availableAttributes.textureCoordinates = true;

            const MeshData originalMesh = mesh;

            GenerateMissingNormalsAndTangents(mesh, false);

            if (!mesh.availableAttributes.normals ||
                mesh.availableAttributes.tangents)
            {
                std::cerr
                    << "Expected generated normals without generated tangents\n";

                return EXIT_FAILURE;
            }

            for (std::size_t vertexIndex = 0;
                vertexIndex < mesh.vertices.size();
                ++vertexIndex)
            {
                const MeshVertex& vertex =
                    mesh.vertices[vertexIndex];

                if (!NearlyEqual(vertex.normal[0], 0.0f) ||
                    !NearlyEqual(vertex.normal[1], 0.0f) ||
                    !NearlyEqual(vertex.normal[2], 1.0f))
                {
                    std::cerr
                        << "Normal-only generation produced an incorrect normal\n";

                    return EXIT_FAILURE;
                }

                if (vertex.tangent !=
                    originalMesh.vertices[vertexIndex].tangent)
                {
                    std::cerr
                        << "Normal-only generation modified a fallback tangent\n";

                    return EXIT_FAILURE;
                }
            }
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Unexpected mesh-processing exception: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }
}