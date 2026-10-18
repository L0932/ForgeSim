#pragma once

#include "SandboxObject.hpp"
#include "SceneLight.hpp"

#include <optional>
#include <variant>

namespace ForgeSim::Sandbox
{
	struct ObjectSelection
	{
		SandboxObjectId id;
	};

	struct LightSelection
	{
		SceneLightId id;
	};

	class SceneSelection final
	{
	public:
		void SelectObject(SandboxObjectId id) noexcept
		{
			m_Target = ObjectSelection{ id };
		}

		void SelectLight(SceneLightId id) noexcept
		{
			if (id == InvalidSceneLightId)
			{
				Clear();
				return;
			}

			m_Target = LightSelection{ id };
		}

		void Clear() noexcept
		{
			m_Target = std::monostate{};
		}

		[[nodiscard]] bool Empty() const noexcept
		{
			return std::holds_alternative<std::monostate>(m_Target);
		}

		[[nodiscard]] std::optional<SandboxObjectId>
			SelectedObjectId() const noexcept
		{
			if (const auto* selected =
				std::get_if<ObjectSelection>(&m_Target))
			{
				return selected->id;
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<SceneLightId>
			SelectedLightId() const noexcept
		{
			if (const auto* selected =
				std::get_if<LightSelection>(&m_Target))
			{
				return selected->id;
			}

			return std::nullopt;
		}

		[[nodiscard]] bool IsObjectSelected(
			SandboxObjectId id) const noexcept
		{
			const auto selected = SelectedObjectId();

			return selected && *selected == id;
		}

		[[nodiscard]] bool IsLightSelected(
			SceneLightId id) const noexcept
		{
			const auto selected = SelectedLightId();

			return selected && *selected == id;
		}

	private:
		std::variant<
			std::monostate,
			ObjectSelection,
			LightSelection> m_Target;
	};
}