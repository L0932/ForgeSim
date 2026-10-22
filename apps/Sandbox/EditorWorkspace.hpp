#pragma once

#include <string>

namespace ForgeSim::Sandbox
{
    class EditorWorkspace final
    {
    public:
        // Call after creating the ImGui context, before the first frame.
        void Initialize();

        // Returns true when the default layout was applied.
        [[nodiscard]] bool Draw();

    private:
        void BuildDefaultLayout(unsigned int dockspaceId);

        // ImGui retains a pointer to this string.
        std::string m_SettingsPath;
    };
}