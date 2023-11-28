#include <darmok/window.hpp>
#include <array>

namespace darmok
{
	static std::array<WindowState, DARMOK_CONFIG_MAX_WINDOWS> s_windows;

	void WindowState::clear()
	{
		handle = { 0 };
		size = WindowSize(0, 0);
		pos = WindowPosition(0, 0);
		nativeHandle = nullptr;
		dropFile = "";
	}

	WindowState& getWindowState(WindowHandle handle)
	{
		return s_windows[handle.idx];
	}
}
