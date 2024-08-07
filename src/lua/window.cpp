#include "window.hpp"
#include "utils.hpp"
#include <darmok/window.hpp>

namespace darmok
{
	LuaWindow::LuaWindow(Window& win) noexcept
		: _win(win)
	{
		win.addListener(*this);
	}

	LuaWindow::~LuaWindow()
	{
		_win.get().removeListener(*this);
	}

	void LuaWindow::onWindowSize(const glm::uvec2& size)
	{
		static const std::string desc("on window size");
		for (auto& listener : _listeners[ListenerType::Size])
		{
			LuaUtils::checkResult(desc, listener.call(size));
		}
	}

	void LuaWindow::onWindowPixelSize(const glm::uvec2& size)
	{
		static const std::string desc("on window pixel size");
		for (auto& listener : _listeners[ListenerType::PixelSize])
		{
			LuaUtils::checkResult(desc, listener.call(size));
		}
	}

	void LuaWindow::onWindowPhase(WindowPhase phase)
	{
		static const std::string desc("on window phase");
		for (auto& listener : _listeners[ListenerType::Phase])
		{
			LuaUtils::checkResult(desc, listener.call(phase));
		}
	}

	void LuaWindow::onWindowVideoMode(const VideoMode& mode)
	{
		static const std::string desc("on window video mode");
		for (auto& listener : _listeners[ListenerType::VideoMode])
		{
			LuaUtils::checkResult(desc, listener.call(mode));
		}
	}

	void LuaWindow::onWindowCursorMode(WindowCursorMode phase)
	{
		static const std::string desc("on window cursor mode");
		for (auto& listener : _listeners[ListenerType::CursorMode])
		{
			LuaUtils::checkResult(desc, listener.call(phase));
		}
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

	glm::vec2 LuaWindow::screenToWindowPoint(const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win.get().screenToWindowPoint(LuaGlm::tableGet(point));
	}

	glm::vec2 LuaWindow::windowToScreenPoint(const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _win.get().windowToScreenPoint(LuaGlm::tableGet(point));
	}

	void LuaWindow::addListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		_listeners[type].push_back(func);
	}

	bool LuaWindow::removeListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		auto itr = _listeners.find(type);
		if (itr == _listeners.end())
		{
			return false;
		}
		auto& listeners = itr->second;
		auto itr2 = std::remove(listeners.begin(), listeners.end(), func);
		if (itr2 == listeners.end())
		{
			return false;
		}
		listeners.erase(itr2, listeners.end());
		return true;
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

		lua.new_enum<LuaWindowListenerType>("WindowListenerType", {
			{ "Position", LuaWindowListenerType::Size },
			{ "PixelSize", LuaWindowListenerType::PixelSize },
			{ "Phase", LuaWindowListenerType::Phase },
			{ "VideoMode", LuaWindowListenerType::VideoMode },
			{ "CursorMode", LuaWindowListenerType::CursorMode },
		});

		lua.new_usertype<LuaWindow>("Window", sol::no_constructor,
			"size", sol::property(&LuaWindow::getSize),
			"pixel_size", sol::property(&LuaWindow::getPixelSize),
			"video_mode", sol::property(&LuaWindow::getVideoMode, &LuaWindow::setVideoMode),
			"video_mode_info", sol::property(&LuaWindow::getVideoModeInfo),
			"cursor_mode", sol::property(&LuaWindow::getCursorMode, &LuaWindow::setCursorMode),
			"screen_to_window_point", &LuaWindow::screenToWindowPoint,
			"window_to_screen_point", &LuaWindow::windowToScreenPoint,
			"add_listener", &LuaWindow::addListener,
			"remove_listener", &LuaWindow::removeListener
		);
	}

}