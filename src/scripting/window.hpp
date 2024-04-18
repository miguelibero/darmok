#pragma once

#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"

namespace darmok
{
    class Window;

	class LuaWindow
	{
	public:
		LuaWindow(Window& win) noexcept;
		const glm::uvec2& getSize() const noexcept;
		glm::uvec2 getPixelSize() const noexcept;
		const glm::ivec4& getViewport() const noexcept;
		const Window& getReal() const noexcept;
		Window& getReal() noexcept;
		glm::uvec2 screenPointToWindow(const glm::vec2& point) const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Window> _win;
	};
}