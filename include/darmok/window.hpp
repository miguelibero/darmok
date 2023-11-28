/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */

#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <darmok/utils.hpp>

#define DARMOK_WINDOW_FLAG_NONE         UINT32_C(0x00000000)
#define DARMOK_WINDOW_FLAG_ASPECT_RATIO UINT32_C(0x00000001)
#define DARMOK_WINDOW_FLAG_FRAME        UINT32_C(0x00000002)

#ifndef DARMOK_CONFIG_MAX_WINDOWS
#	define DARMOK_CONFIG_MAX_WINDOWS 8
#endif // DARMOK_CONFIG_MAX_WINDOWS

namespace darmok
{
	struct WindowHandle final
	{
		uint16_t idx;

		bool operator==(const WindowHandle& other) const
		{
			return idx == other.idx;
		}

		bool operator<(const WindowHandle& other) const
		{
			return idx < other.idx;
		}
	};

	///
	constexpr WindowHandle kDefaultWindowHandle = { 0 };

	///
	enum class WindowSuspendPhase
	{
		WillSuspend,
		DidSuspend,
		WillResume,
		DidResume,

		Count
	};

	/// 
	struct WindowPosition
	{
		int32_t x;
		int32_t y;

		WindowPosition(int32_t vx = 0, int32_t vy = 0)
			: x(vx)
			, y(vy)
		{
		}

		bool operator==(const WindowPosition& other) const
		{
			return x == other.x && y == other.y;
		}
	};

	/// 
	struct WindowSize
	{
		uint32_t width;
		uint32_t height;

		WindowSize(int32_t vwidth = 0, int32_t vheight = 0)
			: width(vwidth)
			, height(vheight)
		{
		}

		bool operator==(const WindowSize& other) const
		{
			return width == other.width && height == other.height;
		}
	};

	/// 
	struct WindowState
	{
		WindowHandle handle;
		WindowPosition pos;
		WindowSize size;
		uint32_t flags;
		void* nativeHandle;
		bool fullScreen;
		std::string dropFile;

		void clear();
	};

	struct WindowCreationOptions
	{
		WindowSize size;
		std::string title = "";
		bool setPos;
		WindowPosition pos;
		uint32_t flags = DARMOK_WINDOW_FLAG_NONE;
	};

	///
	WindowHandle createWindow(const WindowCreationOptions& options);

	///
	void destroyWindow(WindowHandle handle);

	///
	void setWindowPos(WindowHandle handle, const WindowPosition& pos);

	///
	void setWindowSize(WindowHandle handle, const WindowSize& size);

	///
	void setWindowTitle(WindowHandle handle, const std::string& title);

	///
	void setWindowFlags(WindowHandle handle, uint32_t flags, bool enabled);

	///
	void toggleFullscreen(WindowHandle handle);

	///
	void setMouseLock(WindowHandle handle, bool lock);

	///
	void* getNativeWindowHandle(WindowHandle handle);

	///
	void* getNativeDisplayHandle();

	///
	bgfx::NativeWindowHandleType::Enum getNativeWindowHandleType(WindowHandle handle);

	///
	WindowState& getWindowState(WindowHandle handle);

} // namespace darmok


template<> struct std::hash<darmok::WindowHandle> {
	std::size_t operator()(darmok::WindowHandle const& handle) const noexcept {
		return handle.idx;
	}
};