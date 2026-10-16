#pragma once

#include "SandboxObject.hpp"

#include <vector>
#include <span>

namespace ForgeSim::Sandbox
{
	class SandboxScene final
	{
	public:
		SandboxScene();

		[[nodiscard]] std::span<SandboxObject>
			Objects() noexcept;

		[[nodiscard]] std::span<const SandboxObject>
			Objects() const noexcept;

		[[nodiscard]] SandboxObject* FindObject(
			SandboxObjectId objectId) noexcept;

		[[nodiscard]] const SandboxObject* FindObject(
			SandboxObjectId objectId) const noexcept;

	private:
		std::vector<SandboxObject> m_Objects;
	};
}