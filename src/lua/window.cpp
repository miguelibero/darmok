#include "lua/window.hpp"
#include "lua/utils.hpp"
#include <darmok/window.hpp>

namespace darmok
{
	LuaWindowListener::LuaWindowListener(const sol::table& table) noexcept
		: _table(table)
	{
	}

	sol::object LuaWindowListener::getReal() const noexcept
	{
		return _table;
	}

	void LuaWindowListener::onWindowSize(const glm::uvec2& size)
	{
		_sizeDelegate(_table, size);
	}

	void LuaWindowListener::onWindowPixelSize(const glm::uvec2& size)
	{
		_pixelSizeDelegate(_table, size);
	}

	void LuaWindowListener::onWindowPhase(WindowPhase phase)
	{
		_phaseDelegate(_table, phase);
	}

	void LuaWindowListener::onWindowVideoMode(const VideoMode& mode)
	{
		_videoModeDelegate(_table, mode);
	}

	void LuaWindowListener::onWindowCursorMode(WindowCursorMode mode)
	{
		_cursorModeDelegate(_table, mode);
	}

	const LuaTableDelegateDefinition LuaWindowListener::_sizeDelegate{ "on_window_size", "running window size listener" };
	const LuaTableDelegateDefinition LuaWindowListener::_pixelSizeDelegate{ "on_window_pixel_size", "running window pixel size listener" };
	const LuaTableDelegateDefinition LuaWindowListener::_phaseDelegate{ "on_window_phase", "running window phase listener" };
	const LuaTableDelegateDefinition LuaWindowListener::_videoModeDelegate{ "on_window_video_mode", "running window video mode listener" };
	const LuaTableDelegateDefinition LuaWindowListener::_cursorModeDelegate{ "on_window_cursor_mode", "running window cursor mode listener" };

	LuaWindowListenerFilter::LuaWindowListenerFilter(const sol::table& table) noexcept
		: _table(table)
		, _type(entt::type_hash<LuaWindowListener>::value())
	{
	}

	bool LuaWindowListenerFilter::operator()(const IWindowListener& listener) const noexcept
	{
		return listener.getWindowListenerType() == _type && static_cast<const LuaWindowListener&>(listener).getReal() == _table;
	}

	glm::vec2 LuaWindow::screenToWindowPoint(const Window& win, const VarLuaTable<glm::vec2>& point) noexcept
	{
		return win.screenToWindowPoint(LuaGlm::tableGet(point));
	}

	glm::vec2 LuaWindow::windowToScreenPoint(const Window& win, const VarLuaTable<glm::vec2>& point) noexcept
	{
		return win.windowToScreenPoint(LuaGlm::tableGet(point));
	}

	void LuaWindow::addListener(Window& win, const sol::table& table) noexcept
	{
		win.addListener(std::make_unique<LuaWindowListener>(table));
	}

	bool LuaWindow::removeListener(Window& win, const sol::table& table) noexcept
	{
		return win.removeListeners(LuaWindowListenerFilter(table)) > 0;
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

		lua.new_usertype<VideoMode>("VideoMode",
			sol::factories(
				[]() { return VideoMode(); },
				[](const VarLuaTable<glm::uvec2>& size) { return VideoMode{ .size = LuaGlm::tableGet(size) }; },
				[](WindowScreenMode mode) { return VideoMode{ mode }; },
				[](WindowScreenMode mode, const VarLuaTable<glm::uvec2>& size) {
					return VideoMode{ mode, LuaGlm::tableGet(size) }; }
			),
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

		lua.new_usertype<Window>("Window", sol::no_constructor,
			"size", sol::property(&Window::getSize),
			"pixel_size", sol::property(&Window::getPixelSize),
			"framebuffer_scale", sol::property(&Window::getFramebufferScale),
			"video_mode", sol::property(&Window::getVideoMode, &Window::requestVideoMode),
			"video_mode_info", sol::property(&Window::getVideoModeInfo),
			"cursor_mode", sol::property(&Window::getCursorMode, &Window::requestCursorMode),
			"screen_to_window_point", &LuaWindow::screenToWindowPoint,
			"window_to_screen_point", &LuaWindow::windowToScreenPoint,
			"add_listener", &LuaWindow::addListener,
			"remove_listener", &LuaWindow::removeListener
		);
	}

}