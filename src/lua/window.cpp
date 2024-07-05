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

	glm::uvec2 LuaWindow::screenToWindowPoint(const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win->screenToWindowPoint(LuaGlm::tableGet(point));
	}

	glm::vec2 LuaWindow::windowToScreenPoint(const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win->windowToScreenPoint(LuaGlm::tableGet(point));
	}

	void LuaWindow::setCursorMode(WindowCursorMode mode)
	{
		_win->requestCursorMode(mode);
	}

	void LuaWindow::setVideoMode(const VideoMode& mode)
	{
		_win->requestVideoMode(mode);
	}

	const Window& LuaWindow::getReal() const noexcept
	{
		return _win.value();
	}

	Window& LuaWindow::getReal() noexcept
	{
		return _win.value();
	}

	void LuaWindow::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<WindowScreenMode>("WindowScreenMode", {
			{ "Normal", WindowScreenMode::Normal },
			{ "Fullscreen", WindowScreenMode::Fullscreen },
			{ "WindowedFullscreen", WindowScreenMode::WindowedFullscreen }
		});

		lua.new_enum<WindowCursorMode>("WindowCursorMode", {
			{ "Normal", WindowCursorMode::Normal },
			{ "Disabled", WindowCursorMode::Disabled },
			{ "Hidden", WindowCursorMode::Hidden }
		});

		lua.new_usertype<VideoMode>("VideoMode", sol::default_constructor,
			"size", &VideoMode::size,
			"depth", &VideoMode::depth,
			"refreshRate", &VideoMode::refreshRate,
			"monitor", &VideoMode::monitor
		);

		lua.new_usertype<LuaWindow>("Window", sol::no_constructor,
			"size", sol::property(&LuaWindow::getSize),
			"pixel_size", sol::property(&LuaWindow::getPixelSize),
			"video_mode", sol::property(&LuaWindow::setVideoMode),
			"cursor_mode", sol::property(&LuaWindow::setCursorMode),
			"screen_to_window_point", &LuaWindow::screenToWindowPoint,
			"window_to_screen_point", &LuaWindow::windowToScreenPoint
		);
	}

}