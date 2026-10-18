#include "SceneSelectionValidation.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace
{
	using namespace ForgeSim::Sandbox;

	void Require(bool condition, const char* message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void TestEmptySelection()
	{
		const SandboxScene scene;
		SceneSelection selection;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"Empty selection should remain empty.");
	}

	void TestExistingObject()
	{
		const SandboxScene scene;
		SceneSelection selection;

		const SandboxObjectId id =
			scene.Objects().front().id;

		selection.SelectObject(id);

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsObjectSelected(id),
			"Existing object selection should be preserved.");
	}

	void TestMissingObjectWithMatchingLightId()
	{
		SandboxScene scene;
		SceneSelection selection;

		const SandboxObjectId id =
			scene.Objects().front().id;

		scene.MainDirectionalLight().id = id;
		selection.SelectObject(id);

		// Simulate the selected object disappearing.
		scene.Objects().front().id = id + 1;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"A matching light ID must not preserve "
			"a missing object selection.");
	}

	void TestExistingDirectionalLight()
	{
		SandboxScene scene;
		SceneSelection selection;

		const SceneLightId id =
			scene.MainDirectionalLight().id;

		selection.SelectLight(id);

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Existing directional light should remain selected.");

		scene.MainDirectionalLight().enabled = false;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Disabling a directional light must preserve selection.");
	}

	void TestExistingPointLight()
	{
		SandboxScene scene;
		SceneSelection selection;

		const SceneLightId id = scene.AddPointLight();

		selection.SelectLight(id);

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Existing point light should remain selected.");

		scene.PointLights().back().enabled = false;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Disabling a point light must preserve selection.");

		// Replacing the scene removes the added light.
		scene = SandboxScene{};

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"Missing point light selection should be cleared.");
	}

	void TestExistingSpotLight()
	{
		SandboxScene scene;
		SceneSelection selection;

		const SceneLightId id = scene.AddSpotLight();

		selection.SelectLight(id);

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Existing spot light should remain selected.");

		scene.SpotLights().back().enabled = false;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.IsLightSelected(id),
			"Disabling a spot light must preserve selection.");

		scene = SandboxScene{};

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"Missing spot light selection should be cleared.");
	}

	void TestMissingLightWithMatchingObjectId()
	{
		SandboxScene scene;
		SceneSelection selection;

		const SceneLightId id =
			scene.MainDirectionalLight().id;

		scene.Objects().front().id = id;
		selection.SelectLight(id);

		// Simulate the selected directional light disappearing.
		scene.MainDirectionalLight().id = id + 1;

		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"A matching object ID must not preserve "
			"a missing light selection.");

		// Repeated validation should remain safe.
		ClearMissingSceneSelection(selection, scene);

		Require(
			selection.Empty(),
			"Cleared selection should remain empty.");
	}
}

int main()
{
	try
	{
		TestEmptySelection();
		TestExistingObject();
		TestMissingObjectWithMatchingLightId();
		TestExistingDirectionalLight();
		TestExistingPointLight();
		TestExistingSpotLight();
		TestMissingLightWithMatchingObjectId();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}