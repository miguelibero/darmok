#pragma once

#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <sol/sol.hpp>

namespace darmok
{
	class LuaWindow
	{
	public:
		LuaWindow(Window& win) noexcept;
		const glm::uvec2& getSize() const noexcept;
		glm::uvec2 getPixelSize() const noexcept;
		const Window& getReal() const noexcept;
		Window& getReal() noexcept;
		glm::uvec2 screenPointToWindow(const glm::vec2& point) const noexcept;
		void setCursorMode(WindowCursorMode mode);
		void setMode(WindowMode mode);

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Window> _win;
	};
}