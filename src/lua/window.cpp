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
		return _win.get().getSize();
	}

	const glm::uvec2& LuaWindow::getPixelSize() const noexcept
	{
		return _win.get().getPixelSize();
	}

	const VideoMode& LuaWindow::getVideoMode() const noexcept
	{
		return _win.get().getVideoMode();
	}

	void LuaWindow::setVideoMode(const VideoMode& mode) noexcept
	{
		return _win.get().requestVideoMode(mode);
	}

	const VideoModeInfo& LuaWindow::getVideoModeInfo() noexcept
	{
		return _win.get().getVideoModeInfo();
	}

	WindowCursorMode LuaWindow::getCursorMode() const noexcept
	{
		return _win.get().getCursorMode();
	}

	void LuaWindow::setCursorMode(WindowCursorMode mode) const noexcept
	{
		_win.get().requestCursorMode(mode);
	}

	glm::vec2 LuaWindow::screenToWindowPoint(const Window& win, const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win.get().screenToWindowPoint(LuaGlm::tableGet(point));
	}

	glm::vec2 LuaWindow::windowToScreenPoint(const Window& win, const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win.get().windowToScreenPoint(LuaGlm::tableGet(point));
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

		lua.new_usertype<MonitorInfo>("MonitorInfo", sol::default_constructor,
			"size", &MonitorInfo::name,
			"workarea", &MonitorInfo::workarea
		);

		lua.new_usertype<VideoModeInfo>("VideoModeInfo", sol::default_constructor,
			"modes", &VideoModeInfo::modes,
			"monitors", &VideoModeInfo::monitors
		);

		lua.new_usertype<LuaWindow>("Window", sol::no_constructor,
			"size", sol::property(&LuaWindow::getSize),
			"pixel_size", sol::property(&LuaWindow::getPixelSize),
			"video_mode", sol::property(&LuaWindow::getVideoMode, &LuaWindow::setVideoMode),
			"video_mode_info", sol::property(&LuaWindow::getVideoModeInfo),
			"cursor_mode", sol::property(&LuaWindow::getCursorMode, &LuaWindow::setCursorMode),
			"screen_to_window_point", &LuaWindow::screenToWindowPoint,
			"window_to_screen_point", &LuaWindow::windowToScreenPoint
		);
	}

}