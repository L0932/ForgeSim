#pragma once
#include <optional>

namespace ForgeSim::Sandbox
{
    enum class ScreenshotTarget { Scene, Editor };
    class ScreenshotRequest final
    {
    public:
        bool Request(ScreenshotTarget target)
        {
            if(m_Target) return false;
            m_Target=target;m_Delay=1;return true;
        }
        [[nodiscard]] bool Pending() const noexcept { return m_Target.has_value(); }
        [[nodiscard]] std::optional<ScreenshotTarget> EndFrame() noexcept
        {
            if(!m_Target) return std::nullopt;
            // Give popup menus one full frame to close before editor capture.
            if(m_Delay>0) { --m_Delay;return std::nullopt; }
            const auto result=m_Target;m_Target.reset();return result;
        }
    private:
        std::optional<ScreenshotTarget> m_Target;
        int m_Delay=0;
    };
}
