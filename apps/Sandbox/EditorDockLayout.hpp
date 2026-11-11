#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace ForgeSim::Sandbox
{
    // Preserve the current dock proportions when the host window changes size.
    // ImGui otherwise gives the size difference to the central Scene alone.
    inline void ResizeEditorDockLayout(ImGuiID dockspaceId, ImVec2 availableSize)
    {
        ImGuiDockNode* root = ImGui::DockBuilderGetNode(dockspaceId);
        if (!root || root->Size.x <= 0.0f || root->Size.y <= 0.0f ||
            availableSize.x <= 0.0f || availableSize.y <= 0.0f)
            return;

        const bool resizeX = ImAbs(availableSize.x - root->Size.x) >= 1.0f;
        const bool resizeY = ImAbs(availableSize.y - root->Size.y) >= 1.0f;
        if (!resizeX && !resizeY)
            return;

        const ImVec2 scale(availableSize.x / root->Size.x,
            availableSize.y / root->Size.y);
        const auto scaleChildren = [&](auto&& self, ImGuiDockNode* parent) -> void
        {
            for (ImGuiDockNode* child : parent->ChildNodes)
            {
                if (!child) continue;
                // Loaded nodes have valid SizeRef before Size is initialized.
                // Scaling Size here would erase their saved proportions.
                if (resizeX) child->SizeRef.x *= scale.x;
                if (resizeY) child->SizeRef.y *= scale.y;
                self(self, child);
            }
        };
        scaleChildren(scaleChildren, root);
        // DockSpaceOverViewport applies the new root size and lays out children.
    }
}
