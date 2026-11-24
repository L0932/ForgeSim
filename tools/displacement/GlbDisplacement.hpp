#pragma once

#include <ForgeSim/Assets/MeshDisplacement.hpp>
#include <filesystem>
#include <string>

namespace ForgeSim::DisplacementPrototype
{
    struct Report
    {
        std::string heightSource;
        unsigned heightBitDepth = 0;
        std::size_t sourceVertices = 0;
        std::size_t vertices = 0;
        std::size_t triangles = 0;
        double generationMilliseconds = 0;
    };

    // Prototype-only GLB bake. Preserves original material/image JSON and binary
    // data. Only an explicitly selected static UV0 triangle primitive is changed.
    [[nodiscard]] Report DisplaceGlb(const std::filesystem::path& input,
        const std::filesystem::path& output, const std::filesystem::path& externalHeight,
        const std::string& embeddedName, unsigned meshIndex, unsigned primitiveIndex,
        unsigned channel, const Assets::MeshDisplacementSettings& settings);
}
