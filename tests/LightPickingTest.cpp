#include "LightPicking.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>

int main()
{
    using ForgeSim::Renderer::Ray;
    using namespace ForgeSim::Sandbox;

    const Ray ray{
        .origin = { 0.0f, 0.0f, 0.0f },
        .direction = { 0.0f, 0.0f, -1.0f }
    };

    // No targets should produce no hit.
    if (PickClosestLight(ray, {}).has_value())
    {
        std::cerr << "Expected no hit for an empty target list\n";
        return EXIT_FAILURE;
    }

    // Choose the nearest marker regardless of input order.
    {
        std::array<LightPickTarget, 2> targets{
            LightPickTarget{
                .id = 2,
                .bounds = {
                    .center = { 0.0f, 0.0f, -8.0f },
                    .radius = 1.0f
                }
            },
            LightPickTarget{
                .id = 3,
                .bounds = {
                    .center = { 0.0f, 0.0f, -4.0f },
                    .radius = 1.0f
                }
            }
        };

        for (int order = 0; order < 2; ++order)
        {
            const auto hit = PickClosestLight(ray, targets);

            if (!hit ||
                hit->id != 3 ||
                std::abs(hit->distance - 3.0f) > 1e-5f)
            {
                std::cerr << "Nearest light marker was not selected\n";
                return EXIT_FAILURE;
            }

            std::reverse(targets.begin(), targets.end());
        }
    }

    // Equal-distance markers should use the smaller stable ID.
    {
        std::array<LightPickTarget, 2> targets{
            LightPickTarget{
                .id = 9,
                .bounds = {
                    .center = { 0.0f, 0.0f, -4.0f },
                    .radius = 1.0f
                }
            },
            LightPickTarget{
                .id = 4,
                .bounds = {
                    .center = { 0.0f, 0.0f, -4.0f },
                    .radius = 1.0f
                }
            }
        };

        for (int order = 0; order < 2; ++order)
        {
            const auto hit = PickClosestLight(ray, targets);

            if (!hit || hit->id != 4)
            {
                std::cerr << "Light-marker tie was not deterministic\n";
                return EXIT_FAILURE;
            }

            std::reverse(targets.begin(), targets.end());
        }
    }

    // Off-ray markers, markers behind the ray, and unassigned IDs
    // should not be selected.
    {
        const std::array<LightPickTarget, 3> targets{
            LightPickTarget{
                .id = 2,
                .bounds = {
                    .center = { 5.0f, 0.0f, -4.0f },
                    .radius = 0.5f
                }
            },
            LightPickTarget{
                .id = 3,
                .bounds = {
                    .center = { 0.0f, 0.0f, 4.0f },
                    .radius = 0.5f
                }
            },
            LightPickTarget{
                .id = InvalidSceneLightId,
                .bounds = {
                    .center = { 0.0f, 0.0f, -2.0f },
                    .radius = 0.5f
                }
            }
        };

        if (PickClosestLight(ray, targets).has_value())
        {
            std::cerr << "An ineligible light marker was selected\n";
            return EXIT_FAILURE;
        }
    }

    // Picking must not modify target identity or bounds.
    {
        const std::array<LightPickTarget, 1> targets{
            LightPickTarget{
                .id = 5,
                .bounds = {
                    .center = { 0.0f, 0.0f, -4.0f },
                    .radius = 0.25f
                }
            }
        };

        const auto hit = PickClosestLight(ray, targets);

        if (!hit ||
            hit->id != 5 ||
            targets[0].id != 5 ||
            targets[0].bounds.center.z != -4.0f ||
            targets[0].bounds.radius != 0.25f)
        {
            std::cerr << "Picking failed or modified its target\n";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}