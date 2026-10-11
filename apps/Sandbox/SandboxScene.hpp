#pragma once

#include "SandboxObject.hpp"

#include <array>
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
		std::array<SandboxObject, 3> m_Objects;
	};
}