#pragma once

#include "SceneDocument.hpp"

#include <string>
#include <string_view>

namespace ForgeSim::Sandbox
{
    // Returns formatted JSON.
    // Throws std::invalid_argument when document validation fails.
    // Other serialization errors propagate as exceptions.
    [[nodiscard]] std::string SerializeSceneDocument(
        const SceneDocument& document);

    // Throws std::invalid_argument for malformed or invalid documents.
    [[nodiscard]] SceneDocument DeserializeSceneDocument(
        std::string_view text);
}