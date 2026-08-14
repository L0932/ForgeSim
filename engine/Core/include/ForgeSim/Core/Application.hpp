#pragma once

namespace ForgeSim::Core
{
	class Application
	{
	public:
		virtual ~Application() = default;
		virtual void Run() = 0;
	};
}