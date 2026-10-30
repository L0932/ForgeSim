#include <ForgeSim/Renderer/DirectionalShadowProjection.hpp>

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

namespace
{
    using ForgeSim::Renderer::AxisAlignedBoundingBox;
    using ForgeSim::Renderer::CreateDirectionalShadowMatrix;

    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    glm::vec3 Project(const glm::mat4& matrix, const glm::vec3& position)
    {
        const glm::vec4 clip = matrix * glm::vec4(position, 1.0f);
        Require(std::isfinite(clip.w) && clip.w > 0.0f,
            "Projection must produce a finite positive W.");
        const glm::vec3 ndc = glm::vec3(clip) / clip.w;
        Require(std::isfinite(ndc.x) && std::isfinite(ndc.y) &&
            std::isfinite(ndc.z), "Projected positions must be finite.");
        return ndc;
    }

    void RequireCoverage(
        const AxisAlignedBoundingBox& bounds,
        const glm::vec3& direction)
    {
        const glm::mat4 matrix = CreateDirectionalShadowMatrix(bounds, direction);
        for (int corner = 0; corner < 8; ++corner)
        {
            const glm::vec3 position{
                (corner & 1) != 0 ? bounds.maximum.x : bounds.minimum.x,
                (corner & 2) != 0 ? bounds.maximum.y : bounds.minimum.y,
                (corner & 4) != 0 ? bounds.maximum.z : bounds.minimum.z
            };
            const glm::vec3 ndc = Project(matrix, position);
            Require(std::abs(ndc.x) <= 1.0001f &&
                std::abs(ndc.y) <= 1.0001f &&
                std::abs(ndc.z) <= 1.0001f,
                "All eight scene corners must lie inside the light projection.");
        }
    }

    void RequireNear(const glm::vec3& a, const glm::vec3& b)
    {
        Require(glm::length(a - b) < 0.0001f,
            "Equivalent scene/light configurations must project consistently.");
    }

    void RequireInvalid(
        const AxisAlignedBoundingBox& bounds,
        const glm::vec3& direction)
    {
        bool rejected = false;
        try
        {
            static_cast<void>(CreateDirectionalShadowMatrix(bounds, direction));
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        Require(rejected, "Invalid shadow projection input must be rejected.");
    }
}

int main()
{
    try
    {
        const AxisAlignedBoundingBox bounds{
            .minimum = { -4.0f, -2.0f, -7.0f },
            .maximum = { 8.0f, 5.0f, 3.0f }
        };
        const std::array directions{
            glm::vec3{ -0.5f, 1.0f, 0.3f },
            glm::vec3{ 0.0f, 1.0f, 0.0f },
            glm::vec3{ 0.0f, -1.0f, 0.0f },
            glm::vec3{ 0.0001f, 1.0f, 0.0001f },
            glm::vec3{ 1.0f, 0.0f, 0.0f },
            glm::vec3{ 0.0f, 0.0f, -1.0f }
        };
        for (const auto& direction : directions)
        {
            RequireCoverage(bounds, direction);
        }

        // Degenerate, planar, and differently scaled scenes must still fit.
        RequireCoverage({ { 2.0f, 3.0f, 4.0f }, { 2.0f, 3.0f, 4.0f } }, directions[0]);
        RequireCoverage({ { -10.0f, 0.0f, -10.0f }, { 10.0f, 0.0f, 10.0f } }, directions[0]);
        RequireCoverage({ { -0.001f, -0.001f, -0.001f }, { 0.001f, 0.001f, 0.001f } }, directions[0]);
        RequireCoverage({ { -100000.0f, -100000.0f, -100000.0f }, { 100000.0f, 100000.0f, 100000.0f } }, directions[0]);

        const glm::vec3 point{ 1.0f, 1.0f, -1.0f };
        const auto matrix = CreateDirectionalShadowMatrix(bounds, directions[0]);
        for (float magnitude : { 1000.0f, 1.0e-30f, 1.0e30f })
        {
            RequireNear(Project(matrix, point), Project(
                CreateDirectionalShadowMatrix(bounds, directions[0] * magnitude), point));
        }

        const glm::vec3 shift{ 100.0f, -50.0f, 30.0f };
        const AxisAlignedBoundingBox translated{
            bounds.minimum + shift, bounds.maximum + shift
        };
        RequireNear(Project(matrix, point), Project(
            CreateDirectionalShadowMatrix(translated, directions[0]), point + shift));

        // A point toward the light must have a smaller depth than a point behind it.
        const glm::vec3 center = (bounds.minimum + bounds.maximum) * 0.5f;
        const glm::vec3 direction = glm::normalize(directions[0]);
        Require(Project(matrix, center + direction).z <
            Project(matrix, center - direction).z,
            "Shadow depth must increase away from the light.");

        // A distant point must fall outside the projection, allowing the
        // shader to recognize that the map provides no coverage there.
        const glm::vec3 outside = Project(matrix, center + direction * 10000.0f);
        Require(std::abs(outside.z) > 1.0f,
            "Distant geometry must be distinguishable from covered geometry.");

        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float infinity = std::numeric_limits<float>::infinity();
        RequireInvalid(bounds, glm::vec3{ 0.0f });
        RequireInvalid(bounds, { nan, 1.0f, 0.0f });
        RequireInvalid(bounds, { 0.0f, infinity, 1.0f });
        for (int axis = 0; axis < 3; ++axis)
        {
            auto invalid = bounds;
            invalid.minimum[axis] = invalid.maximum[axis] + 1.0f;
            RequireInvalid(invalid, directions[0]);
            invalid = bounds;
            invalid.minimum[axis] = nan;
            RequireInvalid(invalid, directions[0]);
            invalid = bounds;
            invalid.maximum[axis] = infinity;
            RequireInvalid(invalid, directions[0]);
        }
        std::cout << "Directional shadow projection checks passed.\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}
