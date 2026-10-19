#include "GizmoTransform.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
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

	bool MatricesMatch(
		const glm::mat4& left,
		const glm::mat4& right)
	{
		for (int column = 0; column < 4; ++column)
		{
			for (int row = 0; row < 4; ++row)
			{
				const float a = left[column][row];
				const float b = right[column][row];

				if (!std::isfinite(a) || !std::isfinite(b))
				{
					return false;
				}

				const float tolerance =
					1e-4f * std::max({
						1.0f,
						std::abs(a),
						std::abs(b)
						});

				if (std::abs(a - b) > tolerance)
				{
					return false;
				}
			}
		}

		return true;
	}

	void TestOperation(GizmoOperation operation)
	{
		Transform original;
		original.position = { 1.0f, 2.0f, 3.0f };
		original.rotationRadians = { 0.2f, -0.4f, 0.6f };
		original.scale = { -2.0f, 3.0f, 0.5f };

		Transform expected = original;

		switch (operation)
		{
		case GizmoOperation::Translate:
			expected.position = { -4.0f, 5.0f, 2.0f };
			break;

		case GizmoOperation::Rotate:
			expected.rotationRadians = {
				-0.5f, 0.7f, -0.3f
			};
			break;

		case GizmoOperation::Scale:
			expected.scale = { -0.75f, 4.0f, 1.5f };
			break;
		}

		const glm::mat4 requested = expected.ModelMatrix();

		Require(
			TryApplyGizmoTransform(
				original,
				requested,
				operation),
			"Valid gizmo transform was rejected.");

		Require(
			MatricesMatch(original.ModelMatrix(), requested),
			"Applied transform does not match the requested matrix.");
	}

	void RequireRejected(
		Transform original,
		const glm::mat4& requested,
		GizmoOperation operation)
	{
		const glm::mat4 before = original.ModelMatrix();

		Require(
			!TryApplyGizmoTransform(
				original,
				requested,
				operation),
			"Invalid or incompatible edit was accepted.");

		Require(
			MatricesMatch(original.ModelMatrix(), before),
			"Rejected edit changed the original transform.");
	}

	void TestRejectedEdits()
	{
		Transform original;
		original.position = { 1.0f, 2.0f, 3.0f };

		glm::mat4 invalid = original.ModelMatrix();
		invalid[0][0] =
			std::numeric_limits<float>::quiet_NaN();

		RequireRejected(
			original,
			invalid,
			GizmoOperation::Rotate);

		invalid = original.ModelMatrix();
		invalid[3][0] =
			std::numeric_limits<float>::infinity();

		RequireRejected(
			original,
			invalid,
			GizmoOperation::Translate);

		// Shear cannot be represented by our TRS transform.
		invalid = original.ModelMatrix();
		invalid[1][0] = 0.5f;

		RequireRejected(
			original,
			invalid,
			GizmoOperation::Scale);

		Transform changedScale = original;
		changedScale.scale = { 2.0f, 1.0f, 1.0f };

		// Translation must not silently apply a scale change.
		RequireRejected(
			original,
			changedScale.ModelMatrix(),
			GizmoOperation::Translate);

		Transform zeroScale = original;
		zeroScale.scale.x = 0.0f;

		RequireRejected(
			original,
			zeroScale.ModelMatrix(),
			GizmoOperation::Scale);
	}
}

int main()
{
	try
	{
		TestOperation(GizmoOperation::Translate);
		TestOperation(GizmoOperation::Rotate);
		TestOperation(GizmoOperation::Scale);
		TestRejectedEdits();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}