/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */

#pragma once

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/filepath.h>
#include <bx/string.h>
#include <string>
#include <memory>
#include <optional>
#include <array>
#include <vector>
#include <darmok/utils.hpp>
#include <darmok/input.hpp>

namespace bx { struct FileReaderI; struct FileWriterI; struct AllocatorI; }

extern "C" int _main_(int argc, char** argv);

#define DARMOK_WINDOW_FLAG_NONE         UINT32_C(0x00000000)
#define DARMOK_WINDOW_FLAG_ASPECT_RATIO UINT32_C(0x00000001)
#define DARMOK_WINDOW_FLAG_FRAME        UINT32_C(0x00000002)

#ifndef DARMOK_CONFIG_IMPLEMENT_MAIN
#	define DARMOK_CONFIG_IMPLEMENT_MAIN 0
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

#if DARMOK_CONFIG_IMPLEMENT_MAIN
#define DARMOK_IMPLEMENT_MAIN(app, ...)                      \
	int32_t _main_(int32_t argc, char** argv)                \
	{                                                        \
		return darmok::runApp<app>(argc, argv, __VA_ARGS__); \
	}
#else
#define DARMOK_IMPLEMENT_MAIN(app, ...) \
	static app s_app(__VA_ARGS__)
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

namespace darmok
{
	struct WindowHandle final
	{
		uint16_t idx;

	};

	///
	constexpr WindowHandle kDefaultWindowHandle = { 0 };

	///
	enum class SuspendPhase
	{
		WillSuspend,
		DidSuspend,
		WillResume,
		DidResume,

		Count
	};

	///
	const std::string& getKeyName(Key key);

	///
	struct MouseState
	{
		MousePosition pos;
		std::array<uint8_t, to_underlying(MouseButton::Count)> buttons = {};
	};

	///
	struct GamepadState
	{
		std::array<int32_t, to_underlying(GamepadAxis::Count)> axis;
	};

	struct WindowPosition
	{
		int32_t x;
		int32_t y;
	};

	struct WindowSize
	{
		uint32_t width;
		uint32_t height;
	};

	struct WindowState
	{
		WindowHandle handle;
		WindowPosition pos;
		WindowSize size;
		uint32_t flags;
		void* nativeHandle;
		std::string dropFile;

		void clear();
	};

	///
	bool processEvents();

	///
	bx::FileReaderI& getFileReader();

	///
	bx::FileWriterI& getFileWriter();

	///
	bx::AllocatorI& getAllocator();

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
	void setCurrentDir(const std::string& dir);

	WindowState& getWindowState(WindowHandle handle);

	///
	class BX_NO_VTABLE App
	{
	public:

		///
		virtual ~App() = 0;

		///
		virtual void init(const std::vector<std::string>& args) = 0;

		///
		virtual int  shutdown() = 0;

		///
		virtual bool update() = 0;
	};

	///
	int runApp(std::unique_ptr<App>&& app, const std::vector<std::string>& args);

	template<typename T, typename... A>
	int runApp(int argc, const char* const* argv, A... constructArgs)
	{
		auto app = std::make_unique<T>(std::move(constructArgs)...);
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; ++i)
		{
			args[i] = argv[i];
		}
		return runApp(std::move(app), args);
	};
} // namespace entry