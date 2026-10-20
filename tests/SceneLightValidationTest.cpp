#include "SceneLightEditing.hpp"
#include "SceneLightValidation.hpp"

#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string_view>

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

	template<typename Light>
	void ExpectError(
		const Light& light,
		SceneLightKind expectedKind,
		std::string_view expectedProperty)
	{
		const auto error = ValidateLight(light);

		Require(error.has_value(), "Invalid light was accepted.");
		Require(
			error->kind == expectedKind,
			"Validation reported the wrong light kind.");
		Require(
			error->lightId == light.id,
			"Validation reported the wrong light ID.");
		Require(
			error->property == expectedProperty,
			"Validation reported the wrong property.");
		Require(
			!error->message.empty(),
			"Validation must provide a descriptive message.");
	}

	template<typename Light>
	void TestCommonRules(SceneLightKind kind)
	{
		Light light;
		light.id = 42;

		Require(
			!ValidateLight(light),
			"Default light data should be valid.");

		light.intensity = 0.0f;

		Require(
			!ValidateLight(light),
			"Zero intensity is valid even if not submitted.");

		light.color = { 2.0f, 0.5f, 0.0f };
		light.intensity = 3.0f;

		Require(
			!ValidateLight(light),
			"Finite nonnegative HDR color should be valid.");

		light.position.x =
			std::numeric_limits<float>::quiet_NaN();

		ExpectError(light, kind, "position");

		light = Light{};
		light.id = 42;
		light.color.y = -0.1f;

		ExpectError(light, kind, "color");

		light.color.y =
			std::numeric_limits<float>::infinity();

		ExpectError(light, kind, "color");

		light = Light{};
		light.id = 42;
		light.intensity = -1.0f;

		ExpectError(light, kind, "intensity");

		light.intensity =
			std::numeric_limits<float>::quiet_NaN();

		ExpectError(light, kind, "intensity");

		light.intensity =
			std::numeric_limits<float>::infinity();

		ExpectError(light, kind, "intensity");

		light.intensity = 2.0f;
		light.color.x =
			std::numeric_limits<float>::max();

		ExpectError(light, kind, "intensity");

		// Disabled lights still need valid stored properties.
		light = Light{};
		light.id = 42;
		light.enabled = false;
		light.intensity = -1.0f;

		ExpectError(light, kind, "intensity");
	}

	template<typename Light>
	void TestRangeRules(SceneLightKind kind)
	{
		Light light;
		light.id = 73;

		light.range = 0.0f;
		ExpectError(light, kind, "range");

		light.range = -1.0f;
		ExpectError(light, kind, "range");

		light.range =
			std::numeric_limits<float>::infinity();
		ExpectError(light, kind, "range");

		light.range =
			std::numeric_limits<float>::quiet_NaN();
		ExpectError(light, kind, "range");

		light.range = 1e-6f;

		Require(
			!ValidateLight(light),
			"Small positive finite ranges must remain supported.");
	}

	void TestDirectionalDirection()
	{
		DirectionalLight light;
		light.id = 81;

		light.directionToLight = { 2.0f, -3.0f, 4.0f };
		const glm::vec3 before = light.directionToLight;

		Require(
			!ValidateLight(light),
			"Non-unit direction should be accepted.");
		Require(
			light.directionToLight == before,
			"Validation must not normalize scene data.");

		light.directionToLight = glm::vec3{ 0.0f };
		ExpectError(
			light,
			SceneLightKind::Directional,
			"directionToLight");

		light.directionToLight = { 1e-8f, 0.0f, 0.0f };
		ExpectError(
			light,
			SceneLightKind::Directional,
			"directionToLight");

		light.directionToLight.x =
			std::numeric_limits<float>::infinity();
		ExpectError(
			light,
			SceneLightKind::Directional,
			"directionToLight");
	}

	void TestSpotDirection()
	{
		SpotLight light;
		light.id = 82;
		light.emissionDirection = { 0.0f, -5.0f, 0.0f };

		const glm::vec3 before = light.emissionDirection;

		Require(
			!ValidateLight(light),
			"Non-unit spot direction should be accepted.");
		Require(
			light.emissionDirection == before,
			"Validation must preserve the spot direction.");

		light.emissionDirection = glm::vec3{ 0.0f };
		ExpectError(
			light,
			SceneLightKind::Spot,
			"emissionDirection");

		light.emissionDirection = { 0.0f, -1e-8f, 0.0f };
		ExpectError(
			light,
			SceneLightKind::Spot,
			"emissionDirection");

		light.emissionDirection.y =
			std::numeric_limits<float>::quiet_NaN();
		ExpectError(
			light,
			SceneLightKind::Spot,
			"emissionDirection");
	}

	void TestSpotCones()
	{
		SpotLight light;
		light.id = 83;

		light.innerConeAngleRadians = 0.0f;

		Require(
			!ValidateLight(light),
			"An inner angle of zero should be valid.");

		light.innerConeAngleRadians = -0.1f;
		ExpectError(
			light,
			SceneLightKind::Spot,
			"innerConeAngleRadians");

		light.innerConeAngleRadians =
			std::numeric_limits<float>::quiet_NaN();
		ExpectError(
			light,
			SceneLightKind::Spot,
			"innerConeAngleRadians");

		light = SpotLight{};
		light.id = 83;

		light.outerConeAngleRadians =
			light.innerConeAngleRadians;
		ExpectError(
			light,
			SceneLightKind::Spot,
			"outerConeAngleRadians");

		light.outerConeAngleRadians = 0.1f;
		ExpectError(
			light,
			SceneLightKind::Spot,
			"outerConeAngleRadians");

		light.outerConeAngleRadians =
			std::numbers::pi_v<float> *0.5f;
		ExpectError(
			light,
			SceneLightKind::Spot,
			"outerConeAngleRadians");

		light.outerConeAngleRadians =
			std::numeric_limits<float>::infinity();
		ExpectError(
			light,
			SceneLightKind::Spot,
			"outerConeAngleRadians");

		// Ordered angles can still produce identical float cosines.
		light.innerConeAngleRadians = 0.0f;
		light.outerConeAngleRadians = 1e-8f;
		ExpectError(
			light,
			SceneLightKind::Spot,
			"outerConeAngleRadians");
	}

	template<typename Light>
	void TestEditAcceptance(SceneLightKind kind)
	{
		Light target;
		target.id = 100;

		Light candidate = target;
		candidate.enabled = false;
		candidate.position = { 1.0f, 2.0f, 3.0f };
		candidate.color = { 0.25f, 0.5f, 1.0f };
		candidate.intensity = 0.0f;

		Require(
			!TryApplyLightEdit(target, candidate),
			"Valid edit was rejected.");

		Require(
			target.id == 100 &&
			target.enabled == candidate.enabled &&
			target.position == candidate.position &&
			target.color == candidate.color &&
			target.intensity == candidate.intensity,
			"Accepted edit did not update the target.");

		const Light before = target;

		// Include several valid changes alongside one invalid field.
		// None of them should reach the target.
		candidate.enabled = true;
		candidate.position = { 9.0f, 8.0f, 7.0f };
		candidate.color = { 1.0f, 0.0f, 0.0f };
		candidate.intensity = -1.0f;

		const auto error = TryApplyLightEdit(target, candidate);

		Require(
			error &&
			error->kind == kind &&
			error->lightId == target.id &&
			error->property == "intensity",
			"Rejected edit reported incorrect error details.");

		Require(
			target.id == before.id &&
			target.enabled == before.enabled &&
			target.position == before.position &&
			target.color == before.color &&
			target.intensity == before.intensity,
			"Rejected edit partially modified the target.");

		candidate = target;
		candidate.id = 200;
		candidate.position = { 4.0f, 5.0f, 6.0f };

		const auto idError = TryApplyLightEdit(target, candidate);

		Require(
			idError &&
			idError->kind == kind &&
			idError->lightId == before.id &&
			idError->property == "id",
			"Changing a light ID must be rejected.");

		Require(
			target.id == before.id &&
			target.position == before.position,
			"Rejected ID edit modified the target.");
	}
}

int main()
{
	try
	{
		TestCommonRules<DirectionalLight>(
			SceneLightKind::Directional);
		TestCommonRules<PointLight>(
			SceneLightKind::Point);
		TestCommonRules<SpotLight>(
			SceneLightKind::Spot);

		TestRangeRules<PointLight>(SceneLightKind::Point);
		TestRangeRules<SpotLight>(SceneLightKind::Spot);

		TestDirectionalDirection();
		TestSpotDirection();
		TestSpotCones();

		TestEditAcceptance<DirectionalLight>(
			SceneLightKind::Directional);

		TestEditAcceptance<PointLight>(
			SceneLightKind::Point);

		TestEditAcceptance<SpotLight>(
			SceneLightKind::Spot);

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}