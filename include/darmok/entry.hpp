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
#include <darmok/utils.hpp>

namespace bx { struct FileReaderI; struct FileWriterI; struct AllocatorI; }

extern "C" int _main_(int argc, char** argv);

#define ENTRY_WINDOW_FLAG_NONE         UINT32_C(0x00000000)
#define ENTRY_WINDOW_FLAG_ASPECT_RATIO UINT32_C(0x00000001)
#define ENTRY_WINDOW_FLAG_FRAME        UINT32_C(0x00000002)

#ifndef ENTRY_CONFIG_IMPLEMENT_MAIN
#	define ENTRY_CONFIG_IMPLEMENT_MAIN 0
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

#if ENTRY_CONFIG_IMPLEMENT_MAIN
#define ENTRY_IMPLEMENT_MAIN(app, ...)                        \
	int _main_(int argc, char** argv)                         \
	{                                                         \
			auto app std::make_unique<app>(__VA_ARGS__);      \
			return entry::runApp(std::move(app), argc, argv); \
	}
#else
#define ENTRY_IMPLEMENT_MAIN(app, ...) \
	auto ## app ## App(__VA_ARGS__)
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

///
#define ENTRY_HANDLE(name)                                                \
	struct name { uint16_t idx; };                                        \
	inline bool isValid(name handle) { return UINT16_MAX != handle.idx; }

namespace darmok
{
	ENTRY_HANDLE(WindowHandle);
	ENTRY_HANDLE(GamepadHandle);

	///
	constexpr WindowHandle kDefaultWindowHandle = { 0 };

	///
	enum class MouseButton
	{
		None,
		Left,
		Middle,
		Right,

		Count
	};

	///
	enum class GamepadAxis
	{
		LeftX,
		LeftY,
		LeftZ,
		RightX,
		RightY,
		RightZ,

		Count
	};

	///
	enum class Modifier
	{
		None       = 0,
		LeftAlt    = 0x01,
		RightAlt   = 0x02,
		LeftCtrl   = 0x04,
		RightCtrl  = 0x08,
		LeftShift  = 0x10,
		RightShift = 0x20,
		LeftMeta   = 0x40,
		RightMeta  = 0x80,
	};

	///
	enum class Key
	{
		None = 0,
		Esc,
		Return,
		Tab,
		Space,
		Backspace,
		Up,
		Down,
		Left,
		Right,
		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,
		Print,
		Plus,
		Minus,
		LeftBracket,
		RightBracket,
		Semicolon,
		Quote,
		Comma,
		Period,
		Slash,
		Backslash,
		Tilde,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		KeyA,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,

		GamepadA,
		GamepadB,
		GamepadX,
		GamepadY,
		GamepadThumbL,
		GamepadThumbR,
		GamepadShoulderL,
		GamepadShoulderR,
		GamepadUp,
		GamepadDown,
		GamepadLeft,
		GamepadRight,
		GamepadBack,
		GamepadStart,
		GamepadGuide,

		Count
	};

	///
	enum class Suspend
	{
		WillSuspend,
		DidSuspend,
		WillResume,
		DidResume,

		Count
	};

	///
	const std::string& getName(Key _key);

	///
	struct MouseState
	{
		int32_t x = 0;
		int32_t y = 0;
		int32_t z = 0;
		std::array<uint8_t, to_underlying(darmok::MouseButton::Count)> buttons = {};
	};

	///
	struct GamepadState
	{
		std::array<int32_t, to_underlying(GamepadAxis::Count)> axis;
	};

	///
	bool processEvents(uint32_t& width, uint32_t& height, uint32_t& debug, uint32_t& reset);

	///
	bx::FileReaderI& getFileReader();

	///
	bx::FileWriterI& getFileWriter();

	///
	bx::AllocatorI&  getAllocator();

	///
	WindowHandle createWindow(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t flags = ENTRY_WINDOW_FLAG_NONE, const std::string& title = "");

	///
	void destroyWindow(WindowHandle handle);

	///
	void setWindowPos(WindowHandle handle, int32_t x, int32_t y);

	///
	void setWindowSize(WindowHandle handle, uint32_t width, uint32_t height);

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

	///
	struct WindowState
	{
		WindowState()
			: width(0)
			, height(0)
			, nwh(nullptr)
		{
			handle.idx = UINT16_MAX;
		}

		WindowHandle handle;
		uint32_t     width;
		uint32_t     height;
		MouseState   mouse;
		void*        nwh;
		bx::FilePath dropFile;
	};

	///
	bool processWindowEvents(WindowState& state, uint32_t& debug, uint32_t& reset);

	///
	class BX_NO_VTABLE IApp
	{
	public:

		///
		virtual ~IApp() = 0;

		///
		virtual void init(int32_t argc, const char* const* argv, uint32_t width, uint32_t height) = 0;

		///
		virtual int  shutdown() = 0;

		///
		virtual bool update() = 0;

	private:
		BX_ALIGN_DECL(16, uintptr_t) _internal[4];
	};

	///
	int runApp(std::unique_ptr<IApp>&& app, int argc, const char* const* argv);

} // namespace entry