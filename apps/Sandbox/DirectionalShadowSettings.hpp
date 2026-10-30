#pragma once

namespace ForgeSim::Sandbox
{
	struct DirectionalShadowSettings
	{
		bool enabled = true;
		bool filteringEnabled = true;

		float minimumBias = 0.0005f;
		float slopeBias = 0.0025f;
	};
}