#include "SceneBounds.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/vec4.hpp>

namespace ForgeSim::Sandbox
{
    namespace
    {
        [[nodiscard]] bool IsFinite(const glm::vec3& value)
        {
            return std::isfinite(value.x) &&
                std::isfinite(value.y) &&
                std::isfinite(value.z);
        }
    }

    Renderer::AxisAlignedBoundingBox CalculateSceneBounds(
        std::span<const SandboxObject> objects,
        const Renderer::AxisAlignedBoundingBox& modelBounds)
    {
        if (!IsFinite(modelBounds.minimum) ||
            !IsFinite(modelBounds.maximum) ||
            modelBounds.minimum.x > modelBounds.maximum.x ||
            modelBounds.minimum.y > modelBounds.maximum.y ||
            modelBounds.minimum.z > modelBounds.maximum.z)
        {
            throw std::invalid_argument(
                "Cannot frame invalid model bounds.");
        }

        if (objects.empty())
        {
            return modelBounds;
        }

        glm::vec3 minimum{
            std::numeric_limits<float>::max()
        };

        glm::vec3 maximum{
            std::numeric_limits<float>::lowest()
        };

        for (const SandboxObject& object : objects)
        {
            const glm::mat4 matrix = object.transform.ModelMatrix();

            // Transform all eight corners. Transforming only minimum
            // and maximum is incorrect for rotation and negative scale.
            for (unsigned int corner = 0; corner < 8; ++corner)
            {
                const glm::vec3 local{
                    (corner & 1u) != 0
                        ? modelBounds.maximum.x
                        : modelBounds.minimum.x,

                    (corner & 2u) != 0
                        ? modelBounds.maximum.y
                        : modelBounds.minimum.y,

                    (corner & 4u) != 0
                        ? modelBounds.maximum.z
                        : modelBounds.minimum.z
                };

                const glm::vec3 world{
                    matrix * glm::vec4{ local, 1.0f }
                };

                if (!IsFinite(world))
                {
                    throw std::invalid_argument(
                        "Object transform produced non-finite "
                        "scene bounds.");
                }

                minimum = glm::min(minimum, world);
                maximum = glm::max(maximum, world);
            }
        }

        return Renderer::AxisAlignedBoundingBox{
            .minimum = minimum,
            .maximum = maximum
        };
    }
}