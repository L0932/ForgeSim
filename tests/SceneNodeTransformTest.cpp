#include "SceneNodeTransform.hpp"

#include <ForgeSim/Assets/GltfLoader.hpp>

#include <array>
#include <cmath>
#include <filesystem>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <numbers>
#include <stdexcept>

namespace
{
    using ForgeSim::Assets::SceneNodeTransform;
    using ForgeSim::Sandbox::CreateSceneNodeLocalMatrix;

    void RequireNear(const glm::vec4& actual, const glm::vec4& expected,
        const char* message)
    {
        for (glm::length_t i = 0; i < 4; ++i)
        {
            if (!std::isfinite(actual[i]) || std::abs(actual[i] - expected[i]) > 1e-4f)
                throw std::runtime_error(message);
        }
    }

    void CheckTransformContract()
    {
        const glm::vec4 point{1.0f, 2.0f, 3.0f, 1.0f};
        RequireNear(CreateSceneNodeLocalMatrix({}) * point, point,
            "Default node transform must preserve mesh-local coordinates.");

        SceneNodeTransform trs;
        trs.translation = {10.0f, 20.0f, 30.0f};
        trs.scale = {2.0f, 3.0f, 4.0f};
        const float halfAngle = std::numbers::pi_v<float> / 4.0f;
        trs.rotation = {0.0f, 0.0f, std::sin(halfAngle), std::cos(halfAngle)};
        RequireNear(CreateSceneNodeLocalMatrix(trs) * point,
            {4.0f, 22.0f, 42.0f, 1.0f},
            "glTF TRS must scale, rotate, then translate using XYZW quaternion order.");

        SceneNodeTransform matrix;
        matrix.translation = {1000.0f, 1000.0f, 1000.0f};
        matrix.scale = {100.0f, 100.0f, 100.0f};
        matrix.matrix = std::array<float, 16>{
            0.0f, 2.0f, 0.0f, 0.0f,
            -3.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 4.0f, 0.0f,
            10.0f, 20.0f, 30.0f, 1.0f};
        RequireNear(CreateSceneNodeLocalMatrix(matrix) * point,
            {4.0f, 22.0f, 42.0f, 1.0f},
            "Explicit column-major matrix must override TRS.");

        SceneNodeTransform parent;
        parent.translation = {10.0f, 0.0f, 0.0f};
        parent.rotation = trs.rotation;
        parent.scale = {2.0f, 2.0f, 2.0f};
        SceneNodeTransform child;
        child.translation = {3.0f, 0.0f, 0.0f};
        RequireNear(CreateSceneNodeLocalMatrix(parent) *
            CreateSceneNodeLocalMatrix(child) * glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},
            {10.0f, 8.0f, 0.0f, 1.0f},
            "Child transforms must compose after the parent transform.");

        SceneNodeTransform rotate;
        rotate.rotation = trs.rotation;
        RequireNear(CreateSceneNodeLocalMatrix(rotate) *
            glm::vec4{5.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 5.0f, 0.0f, 1.0f},
            "Offset geometry must rotate about the authored origin, without recentering.");

        SceneNodeTransform signedScale;
        signedScale.scale = {-2.0f, 0.5f, 3.0f};
        RequireNear(CreateSceneNodeLocalMatrix(signedScale) * point,
            {-2.0f, 1.0f, 9.0f, 1.0f},
            "Signed and nonuniform node scales must be preserved.");
    }
    void CheckImportedOffsetFixture()
    {
        const auto path = std::filesystem::path{FORGESIM_TEST_ASSET_DIRECTORY}
            / "OffsetHierarchy.gltf";
        const std::array<glm::vec4, 3> meshPositions{{
            {4.0f, 1.0f, 0.0f, 1.0f},
            {6.0f, 1.0f, 0.0f, 1.0f},
            {4.0f, 3.0f, 0.0f, 1.0f}}};
        const auto checkMesh = [&](const ForgeSim::Assets::MeshData& mesh)
        {
            if (mesh.vertices.size() != 3 || mesh.indices.size() != 3 ||
                mesh.indices[0] != 0 || mesh.indices[1] != 1 || mesh.indices[2] != 2)
                throw std::runtime_error("Offset fixture topology was not preserved.");
            for (std::size_t i = 0; i < meshPositions.size(); ++i)
            {
                const auto& p = mesh.vertices[i].position;
                RequireNear({p[0], p[1], p[2], 1.0f}, meshPositions[i],
                    "Importer must preserve offset mesh vertices without baking nodes or recentering.");
            }
        };

        // Both loader APIs must leave the same mesh-local positions intact.
        checkMesh(ForgeSim::Assets::LoadGltfMesh(path));
        const auto scene = ForgeSim::Assets::LoadGltfScene(path);
        if (scene.meshes.size() != 1 || scene.meshes[0].primitives.size() != 1 ||
            scene.nodes.size() != 3 || scene.rootNodeIndices.size() != 2 ||
            scene.rootNodeIndices[0] != 0 || scene.rootNodeIndices[1] != 2 ||
            scene.nodes[0].childIndices.size() != 1 || scene.nodes[0].childIndices[0] != 1 ||
            scene.nodes[0].meshIndex || scene.nodes[1].meshIndex != 0 ||
            scene.nodes[2].meshIndex != 0 || !scene.nodes[2].localTransform.matrix)
            throw std::runtime_error("Offset fixture instances or hierarchy were not preserved.");
        checkMesh(scene.meshes[0].primitives[0].geometry);

        const auto hierarchy = CreateSceneNodeLocalMatrix(scene.nodes[0].localTransform) *
            CreateSceneNodeLocalMatrix(scene.nodes[1].localTransform);
        const auto matrixInstance = CreateSceneNodeLocalMatrix(scene.nodes[2].localTransform);
        const std::array<glm::vec4, 3> hierarchyPositions{{
            {9.0f, 34.0f, 30.0f, 1.0f},
            {9.0f, 38.0f, 30.0f, 1.0f},
            {7.0f, 34.0f, 30.0f, 1.0f}}};
        const std::array<glm::vec4, 3> matrixPositions{{
            {2.0f, 4.0f, 0.0f, 1.0f},
            {4.0f, 4.0f, 0.0f, 1.0f},
            {2.0f, 6.0f, 0.0f, 1.0f}}};
        for (std::size_t i = 0; i < meshPositions.size(); ++i)
        {
            RequireNear(hierarchy * meshPositions[i], hierarchyPositions[i],
                "Loaded parent/child TRS did not produce the authored vertex positions.");
            RequireNear(matrixInstance * meshPositions[i], matrixPositions[i],
                "Loaded explicit matrix did not produce the authored vertex positions.");
        }
    }

}

int main()
{
    try
    {
        CheckTransformContract();
        CheckImportedOffsetFixture();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
