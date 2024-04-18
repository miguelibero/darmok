#include "window.hpp"
#include <darmok/window.hpp>

namespace darmok
{
	LuaWindow::LuaWindow(Window& win) noexcept
		: _win(win)
	{
	}

	const glm::uvec2& LuaWindow::getSize() const noexcept
	{
		return _win->getSize();
	}

	glm::uvec2 LuaWindow::getPixelSize() const noexcept
	{
		return _win->getPixelSize();
	}

	const glm::ivec4& LuaWindow::getViewport() const noexcept
	{
		return _win->getViewport();
	}

	glm::uvec2 LuaWindow::screenPointToWindow(const glm::vec2& point) const noexcept
	{
		return _win->screenPointToWindow(point);
	}

	const Window& LuaWindow::getReal() const noexcept
	{
		return _win.value();
	}

	Window& LuaWindow::getReal() noexcept
	{
		return _win.value();
	}

	void LuaWindow::configure(sol::state_view& lua) noexcept
	{
		auto usertype = lua.new_usertype<LuaWindow>("Window");
		usertype["size"] = sol::property(&LuaWindow::getSize);
		usertype["pixel_size"] = sol::property(&LuaWindow::getPixelSize);
		usertype["viewport"] = sol::property(&LuaWindow::getViewport);
		usertype["screen_point_to_window"] = &LuaWindow::screenPointToWindow;
	}

}