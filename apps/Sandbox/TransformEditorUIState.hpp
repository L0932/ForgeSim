#pragma once

#include "ViewportTransformGizmo.hpp"

namespace ForgeSim::Sandbox
{
	struct TransformEditorUIState
	{
		GizmoOperation operation = GizmoOperation::Translate;
		GizmoSpace space = GizmoSpace::World;

		bool undoRequested = false;
		bool redoRequested = false;
	};
}