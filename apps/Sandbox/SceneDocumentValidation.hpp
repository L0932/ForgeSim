#pragma once

#include "SceneDocument.hpp"

#include <optional>
#include <string>

namespace ForgeSim::Sandbox
{
    struct SceneDocumentValidationError
    {
        std::string property;
        std::string message;
    };

    // Empty optional means valid.
    // Does not modify the document or access the filesystem.
    [[nodiscard]]
    std::optional<SceneDocumentValidationError>
        ValidateSceneDocument(const SceneDocument& document);
}