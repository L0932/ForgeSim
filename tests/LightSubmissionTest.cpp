#include "LightSubmission.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

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

	bool NearlyEqual(float left, float right)
	{
		return std::isfinite(left) &&
			std::isfinite(right) &&
			std::abs(left - right) <= 1e-5f;
	}

	auto Prepare(std::span<const PointLight> lights)
	{
		return PreparePointLights(lights);
	}

	auto Prepare(std::span<const SpotLight> lights)
	{
		return PrepareSpotLights(lights);
	}

	template<typename Light>
	void TestEmptyAndInvalidInputs()
	{
		Require(
			Prepare(std::span<const Light>{}).count == 0,
			"Empty input must produce zero lights.");

		std::array<Light, 10> lights{};

		lights[0].enabled = false;
		lights[1].intensity = 0.0f;
		lights[2].intensity = -1.0f;
		lights[3].range = 0.0f;
		lights[4].range = -1.0f;
		lights[5].color.x = -0.1f;

		lights[6].position.x =
			std::numeric_limits<float>::quiet_NaN();

		lights[7].range =
			std::numeric_limits<float>::infinity();

		lights[8].intensity =
			std::numeric_limits<float>::infinity();

		// Finite inputs whose product overflows.
		lights[9].color.x =
			std::numeric_limits<float>::max();
		lights[9].intensity = 2.0f;

		Require(
			Prepare(std::span<const Light>{ lights }).count == 0,
			"Disabled or invalid lights must not be submitted.");
	}

	template<typename Light>
	void TestCapacityAndOrder()
	{
		constexpr std::size_t capacity =
			std::is_same_v<Light, PointLight>
			? maximumSubmittedPointLights
			: maximumSubmittedSpotLights;

		std::vector<Light> lights;

		for (std::size_t index = 0; index < capacity + 3; ++index)
		{
			Light disabled;
			disabled.enabled = false;
			lights.push_back(disabled);

			Light invalid;
			invalid.range = -1.0f;
			lights.push_back(invalid);

			Light valid;
			valid.position.x = static_cast<float>(index);
			lights.push_back(valid);
		}

		const auto submission =
			Prepare(std::span<const Light>{ lights });

		Require(
			submission.count == capacity,
			"Submission must stop at its capacity.");

		for (std::size_t index = 0; index < capacity; ++index)
		{
			Require(
				submission.lights[index].position.x ==
				static_cast<float>(index),
				"Filtering must preserve valid lights in scene order.");
		}

		for (Light& light : lights)
		{
			light.enabled = false;
		}

		Require(
			Prepare(std::span<const Light>{ lights }).count == 0,
			"Disabling all lights must produce zero submitted lights.");
	}

	void TestPointValues()
	{
		std::array<PointLight, 1> lights{};

		PointLight& light = lights.front();
		light.position = { 1.0f, 2.0f, 3.0f };
		light.color = { 0.25f, 0.5f, 1.0f };
		light.intensity = 4.0f;
		light.range = 8.0f;

		const auto submission = PreparePointLights(lights);

		Require(submission.count == 1, "Valid point light was rejected.");

		const PreparedPointLight& prepared =
			submission.lights.front();

		Require(
			prepared.position == light.position &&
			prepared.range == light.range,
			"Point-light position or range changed.");

		Require(
			prepared.radiance == glm::vec3{ 1.0f, 2.0f, 4.0f },
			"Point-light color and intensity were combined incorrectly.");

		Require(
			light.color == glm::vec3{ 0.25f, 0.5f, 1.0f } &&
			light.intensity == 4.0f,
			"Preparation modified point-light scene data.");
	}

	void TestSpotValues()
	{
		std::array<SpotLight, 1> lights{};

		SpotLight& light = lights.front();
		light.position = { 2.0f, 3.0f, 4.0f };
		light.emissionDirection = { 0.0f, -5.0f, 0.0f };
		light.color = { 0.5f, 0.25f, 1.0f };
		light.intensity = 2.0f;
		light.range = 12.0f;

		// Known angles: 0 and pi/3, with cosines 1 and 0.5.
		light.innerConeAngleRadians = 0.0f;
		light.outerConeAngleRadians = 1.0471975512f;

		const auto submission = PrepareSpotLights(lights);

		Require(submission.count == 1, "Valid spot light was rejected.");

		const PreparedSpotLight& prepared =
			submission.lights.front();

		Require(
			prepared.position == light.position &&
			prepared.range == light.range,
			"Spot-light position or range changed.");

		Require(
			prepared.emissionDirection ==
			glm::vec3{ 0.0f, -1.0f, 0.0f },
			"Submitted spot direction must be normalized.");

		Require(
			prepared.radiance == glm::vec3{ 1.0f, 0.5f, 2.0f },
			"Spot-light color and intensity were combined incorrectly.");

		Require(
			NearlyEqual(prepared.innerConeCos, 1.0f) &&
			NearlyEqual(prepared.outerConeCos, 0.5f),
			"Spot cone angles were converted incorrectly.");

		Require(
			light.emissionDirection ==
			glm::vec3{ 0.0f, -5.0f, 0.0f } &&
			light.innerConeAngleRadians == 0.0f &&
			light.outerConeAngleRadians == 1.0471975512f,
			"Preparation modified spot-light scene data.");
	}

	void TestInvalidSpotGeometry()
	{
		std::array<SpotLight, 7> lights{};

		lights[0].emissionDirection = glm::vec3{ 0.0f };

		lights[1].emissionDirection.x =
			std::numeric_limits<float>::quiet_NaN();

		lights[2].innerConeAngleRadians = -0.1f;

		lights[3].outerConeAngleRadians =
			lights[3].innerConeAngleRadians;

		lights[4].outerConeAngleRadians = 2.0f;

		lights[5].outerConeAngleRadians =
			std::numeric_limits<float>::infinity();

		// Valid angle ordering, but float cosine values collapse
		// to the same value. Reject unsafe smoothstep edges.
		lights[6].innerConeAngleRadians = 0.0f;
		lights[6].outerConeAngleRadians = 1e-8f;

		Require(
			PrepareSpotLights(lights).count == 0,
			"Invalid spot directions or cones must be rejected.");
	}
}

int main()
{
	try
	{
		TestEmptyAndInvalidInputs<PointLight>();
		TestEmptyAndInvalidInputs<SpotLight>();

		TestCapacityAndOrder<PointLight>();
		TestCapacityAndOrder<SpotLight>();

		TestPointValues();
		TestSpotValues();
		TestInvalidSpotGeometry();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}