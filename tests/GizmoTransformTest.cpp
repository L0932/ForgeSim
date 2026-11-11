#include "GizmoTransform.hpp"
#include <glm/geometric.hpp>

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
    void TestCenteredPivot()
    {
        Transform transform;
        transform.position = {10.0f, 20.0f, 30.0f};
        transform.rotationRadians = {0.25f, 0.5f, -0.3f};
        transform.scale = {-2.0f, 3.0f, 0.5f};
        const glm::mat4 original = transform.ModelMatrix();
        if (!transform.TrySetPivot({5.0f, 2.0f, 0.0f}))
            throw std::runtime_error("Could not set a finite pivot.");
        const auto requireSameMatrix = [](const glm::mat4& a, const glm::mat4& b)
        {
            for (int column = 0; column < 4; ++column)
                for (int row = 0; row < 4; ++row)
                    if (std::abs(a[column][row] - b[column][row]) > 1e-4f)
                        throw std::runtime_error("Changing pivot moved the geometry.");
        };
        requireSameMatrix(original, transform.ModelMatrix());
        const glm::vec3 anchor{transform.ModelMatrix() * glm::vec4{transform.pivot, 1.0f}};
        if (glm::length(anchor - transform.position) > 1e-4f)
            throw std::runtime_error("Pivot did not map to the transform position.");

        for (const auto operation : {GizmoOperation::Translate, GizmoOperation::Rotate,
            GizmoOperation::Scale})
        {
            Transform edited = transform;
            if (operation == GizmoOperation::Translate) edited.position.x += 3.0f;
            if (operation == GizmoOperation::Rotate) edited.rotationRadians.y += 0.2f;
            if (operation == GizmoOperation::Scale) edited.scale.y *= 1.5f;
            Transform applied = transform;
            if (!TryApplyGizmoTransform(applied, edited.GizmoMatrix(), operation))
                throw std::runtime_error("Gizmo rejected a transform with a centered pivot.");
            requireSameMatrix(applied.ModelMatrix(), edited.ModelMatrix());
            if (applied.pivot != transform.pivot)
                throw std::runtime_error("Gizmo changed the chosen pivot.");
        }
        if (!transform.TrySetPivot({0.0f, 0.0f, 0.0f}))
            throw std::runtime_error("Could not restore the authored origin.");
        requireSameMatrix(original, transform.ModelMatrix());
        const auto unchanged = transform.ModelMatrix();
        if (transform.TrySetPivot({std::numeric_limits<float>::infinity(), 0.0f, 0.0f}))
            throw std::runtime_error("Non-finite pivot was accepted.");
        requireSameMatrix(unchanged, transform.ModelMatrix());
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
        TestCenteredPivot();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return EXIT_FAILURE;
	}
}