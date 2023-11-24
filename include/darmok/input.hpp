#pragma once

#include <functional>
#include <string>

namespace darmok
{
#ifndef DARMOK_CONFIG_MAX_GAMEPADS
#	define DARMOK_CONFIG_MAX_GAMEPADS 4
#endif // DARMOK_CONFIG_MAX_GAMEPADS

	struct GamepadHandle final
	{
		uint16_t idx;
	};

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
	enum class KeyModifier
	{
		None = 0,
		LeftAlt = 0x01,
		RightAlt = 0x02,
		LeftCtrl = 0x04,
		RightCtrl = 0x08,
		LeftShift = 0x10,
		RightShift = 0x20,
		LeftMeta = 0x40,
		RightMeta = 0x80,
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

	struct InputBinding
	{
		void set(Key key, uint8_t modifiers, uint8_t flags, std::function<void()> fn);
		void clear();

		Key key;
		uint8_t modifiers;
		uint8_t flags;
		std::function<void()> fn;
		std::string name;
	};

	struct MousePosition
	{
		int32_t x;
		int32_t y;
		int32_t z;
	};

	struct NormMousePosition
	{
		float x;
		float y;
		float z;
	};

	struct Utf8Char
	{
		uint8_t len;
		uint32_t data;
	};

	///
	void inputInit();

	///
	void inputShutdown();

	///
	void inputAddBindings(const std::string& name, std::vector<InputBinding>&& bindings);

	///
	void inputRemoveBindings(const std::string& name);

	///
	void inputProcess();

	///
	void inputSetKeyState(Key key, uint8_t modifiers, bool down);

	///
	bool inputGetKeyState(Key key, uint8_t& modifiers);

	///
	uint8_t inputGetModifiersState();

	/// Adds single UTF-8 encoded character into input buffer.
	void inputChar(const Utf8Char& data);

	/// Returns single UTF-8 encoded character from input buffer.
	Utf8Char inputGetChar();

	/// Flush internal input buffer.
	void inputCharFlush();

	///
	void inputSetMouseResolution(uint16_t width, uint16_t height);

	///
	void inputSetMousePos(const MousePosition& pos);

	///
	void inputSetMouseButtonState(MouseButton button, uint8_t state);

	///
	void inputSetMouseLock(bool lock);

	///
	const NormMousePosition& inputGetMouse();

	///
	bool inputIsMouseLocked();

	///
	void inputSetGamepadAxis(GamepadHandle handle, GamepadAxis axis, int32_t value);

	///
	int32_t inputGetGamepadAxis(GamepadHandle handle, GamepadAxis axis);
}

