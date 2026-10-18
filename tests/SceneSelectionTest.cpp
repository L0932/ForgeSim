#include "SceneSelection.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>

int main()
{
    using namespace ForgeSim::Sandbox;

    SceneSelection selection;

    // Default construction should select nothing.
    if (!selection.Empty() ||
        selection.SelectedObjectId().has_value() ||
        selection.SelectedLightId().has_value())
    {
        std::cerr << "Expected an initially empty selection\n";
        return EXIT_FAILURE;
    }

    selection.SelectObject(7);

    if (selection.Empty() ||
        !selection.IsObjectSelected(7) ||
        selection.IsObjectSelected(8) ||
        selection.SelectedObjectId() !=
        std::optional<SandboxObjectId>{ 7 } ||
        selection.SelectedLightId().has_value())
    {
        std::cerr << "Object selection was incorrect\n";
        return EXIT_FAILURE;
    }

    // Identical numeric IDs must remain distinct across target types.
    selection.SelectLight(7);

    if (selection.Empty() ||
        !selection.IsLightSelected(7) ||
        selection.IsLightSelected(8) ||
        selection.IsObjectSelected(7) ||
        selection.SelectedObjectId().has_value() ||
        selection.SelectedLightId() !=
        std::optional<SceneLightId>{ 7 })
    {
        std::cerr << "Light selection did not replace object selection\n";
        return EXIT_FAILURE;
    }

    selection.SelectObject(7);

    if (!selection.IsObjectSelected(7) ||
        selection.IsLightSelected(7) ||
        selection.SelectedLightId().has_value())
    {
        std::cerr << "Object selection did not replace light selection\n";
        return EXIT_FAILURE;
    }

    selection.Clear();
    selection.Clear();

    if (!selection.Empty() ||
        selection.SelectedObjectId().has_value() ||
        selection.SelectedLightId().has_value())
    {
        std::cerr << "Clearing selection did not leave it empty\n";
        return EXIT_FAILURE;
    }

    // An unassigned light ID clears the existing selection.
    selection.SelectObject(9);
    selection.SelectLight(InvalidSceneLightId);

    if (!selection.Empty())
    {
        std::cerr << "Invalid light ID should clear selection\n";
        return EXIT_FAILURE;
    }

    // Selection changes should also work after clearing.
    selection.SelectLight(12);
    selection.SelectLight(12);

    if (!selection.IsLightSelected(12) ||
        selection.SelectedObjectId().has_value())
    {
        std::cerr << "Repeated light selection was incorrect\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}