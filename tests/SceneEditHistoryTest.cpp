#include "SceneEditHistory.hpp"

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

	bool TransformsEqual(
		const Transform& left,
		const Transform& right)
	{
		return left.position == right.position &&
			left.rotationRadians == right.rotationRadians &&
			left.scale == right.scale;
	}

	void TestMixedHistory()
	{
		SandboxScene scene;
		SceneEditHistory history;

		Require(!history.CanUndo(), "New history must be empty.");
		Require(!history.CanRedo(), "New history must have no redo.");
		Require(!history.Undo(scene), "Empty undo must return false.");
		Require(!history.Redo(scene), "Empty redo must return false.");

		SandboxObject& object = scene.Objects().front();

		const Transform objectBefore = object.transform;
		Transform objectAfter = objectBefore;
		objectAfter.position = { 2.0f, 3.0f, 4.0f };
		objectAfter.rotationRadians = { 0.2f, -0.3f, 0.4f };
		objectAfter.scale = { -2.0f, 3.0f, 0.5f };

		object.transform = objectAfter;

		history.RecordApplied(ObjectTransformEdit{
			.objectId = object.id,
			.before = objectBefore,
			.after = objectAfter
			});

		static_cast<void>(scene.AddPointLight());
		PointLight& light = scene.PointLights().back();

		const glm::vec3 lightBefore = light.position;
		const glm::vec3 lightAfter{ 8.0f, 9.0f, 10.0f };

		light.position = lightAfter;

		history.RecordApplied(PointLightPositionEdit{
			.lightId = light.id,
			.before = lightBefore,
			.after = lightAfter
			});

		Require(history.Undo(scene), "Light undo failed.");
		Require(
			light.position == lightBefore,
			"Light undo restored the wrong position.");
		Require(
			TransformsEqual(object.transform, objectAfter),
			"Light undo changed the object.");

		Require(history.Undo(scene), "Object undo failed.");
		Require(
			TransformsEqual(object.transform, objectBefore),
			"Object undo did not restore the full transform.");
		Require(!history.CanUndo(), "Undo should be exhausted.");

		Require(history.Redo(scene), "Object redo failed.");
		Require(
			TransformsEqual(object.transform, objectAfter),
			"Object redo did not restore the full transform.");

		Require(history.Redo(scene), "Light redo failed.");
		Require(
			light.position == lightAfter,
			"Light redo restored the wrong position.");
		Require(!history.CanRedo(), "Redo should be exhausted.");
	}

	void TestNewEditDiscardsRedo()
	{
		SandboxScene scene;
		SceneEditHistory history;
		SandboxObject& object = scene.Objects().front();

		const Transform original = object.transform;
		Transform first = original;
		first.position.x += 1.0f;

		object.transform = first;

		history.RecordApplied(ObjectTransformEdit{
			.objectId = object.id,
			.before = original,
			.after = first
			});

		Require(history.Undo(scene), "Initial undo failed.");

		Transform replacement = original;
		replacement.position.y += 2.0f;
		object.transform = replacement;

		history.RecordApplied(ObjectTransformEdit{
			.objectId = object.id,
			.before = original,
			.after = replacement
			});

		Require(
			!history.CanRedo(),
			"A new edit must discard the redo branch.");
		Require(
			!history.Redo(scene),
			"Discarded edit must not be redone.");
		Require(
			TransformsEqual(object.transform, replacement),
			"Failed redo changed the scene.");

		Require(history.Undo(scene), "Replacement undo failed.");
		Require(
			TransformsEqual(object.transform, original),
			"Replacement undo restored the wrong transform.");

		Require(history.Redo(scene), "Replacement redo failed.");
		Require(
			TransformsEqual(object.transform, replacement),
			"Replacement redo restored the wrong transform.");

		history.Clear();

		Require(!history.CanUndo(), "Clear must remove undo.");
		Require(!history.CanRedo(), "Clear must remove redo.");
	}

	void TestMissingLightDoesNotRestoreObject()
	{
		SandboxScene scene;
		SceneEditHistory history;

		static_cast<void>(scene.AddPointLight());
		PointLight& light = scene.PointLights().back();

		const SceneLightId missingId = light.id;
		const glm::vec3 before = light.position;
		const glm::vec3 after{ 5.0f, 6.0f, 7.0f };

		light.position = after;

		history.RecordApplied(PointLightPositionEdit{
			.lightId = missingId,
			.before = before,
			.after = after
			});

		// Simulate a missing light and an overlapping object ID.
		light.id = missingId + 1;

		SandboxObject& object = scene.Objects().front();
		object.id = missingId;
		const Transform objectBefore = object.transform;

		Require(
			!history.Undo(scene),
			"Undo of a missing target must return false.");
		Require(
			TransformsEqual(object.transform, objectBefore),
			"A matching object ID must not receive a light edit.");
		Require(
			light.position == after,
			"Undo changed a different light.");

		Require(
			!history.Redo(scene),
			"Redo of a missing target must return false.");
		Require(
			TransformsEqual(object.transform, objectBefore),
			"Redo changed an object with a matching ID.");
	}

	void TestSpotAndDirectionalHistory()
	{
		SandboxScene scene;
		SceneEditHistory history;

		static_cast<void>(scene.AddSpotLight());
		SpotLight& spot = scene.SpotLights().back();

		const SpotLightPose spotBefore{
			.position = spot.position,
			.emissionDirection = spot.emissionDirection
		};

		const SpotLightPose spotAfter{
			.position = { 3.0f, 4.0f, 5.0f },
			.emissionDirection = { 1.0f, 0.0f, 0.0f }
		};

		spot.position = spotAfter.position;
		spot.emissionDirection = spotAfter.emissionDirection;

		history.RecordApplied(SpotLightPoseEdit{
			.lightId = spot.id,
			.before = spotBefore,
			.after = spotAfter
			});

		DirectionalLight& directional =
			scene.MainDirectionalLight();

		const glm::vec3 directionBefore =
			directional.directionToLight;

		const glm::vec3 directionAfter{
			0.0f, 0.0f, 1.0f
		};

		directional.directionToLight = directionAfter;

		history.RecordApplied(DirectionalLightDirectionEdit{
			.lightId = directional.id,
			.before = directionBefore,
			.after = directionAfter
			});

		Require(history.Undo(scene), "Directional undo failed.");
		Require(
			directional.directionToLight == directionBefore,
			"Directional undo restored the wrong direction.");
		Require(
			spot.position == spotAfter.position &&
			spot.emissionDirection == spotAfter.emissionDirection,
			"Directional undo changed the spot light.");

		Require(history.Undo(scene), "Spot undo failed.");
		Require(
			spot.position == spotBefore.position &&
			spot.emissionDirection == spotBefore.emissionDirection,
			"Spot undo did not restore its pose.");

		Require(history.Redo(scene), "Spot redo failed.");
		Require(
			spot.position == spotAfter.position &&
			spot.emissionDirection == spotAfter.emissionDirection,
			"Spot redo did not restore its pose.");

		Require(history.Redo(scene), "Directional redo failed.");
		Require(
			directional.directionToLight == directionAfter,
			"Directional redo restored the wrong direction.");
	}
}

int main()
{
	try
	{
		TestMixedHistory();
		TestNewEditDiscardsRedo();
		TestMissingLightDoesNotRestoreObject();
		TestSpotAndDirectionalHistory();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}