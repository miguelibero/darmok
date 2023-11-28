#pragma once

#include <functional>
#include <string>
#include <array>
#include <vector>
#include <darmok/utils.hpp>

namespace darmok
{
#ifndef DARMOK_CONFIG_MAX_GAMEPADS
#	define DARMOK_CONFIG_MAX_GAMEPADS 4
#endif // DARMOK_CONFIG_MAX_GAMEPADS

	struct WindowSize;

	struct GamepadHandle final
	{
		uint16_t idx;

		bool operator==(const GamepadHandle& other) const
		{
			return idx == other.idx;
		}

		bool operator<(const GamepadHandle& other) const
		{
			return idx < other.idx;
		}
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

	struct KeyModifiers
	{
		static constexpr int None = 0;
		static constexpr int Shift = to_underlying(KeyModifier::LeftShift) | to_underlying(KeyModifier::RightShift);
		static constexpr int Ctrl = to_underlying(KeyModifier::LeftCtrl) | to_underlying(KeyModifier::RightCtrl);
		static constexpr int Alt = to_underlying(KeyModifier::LeftAlt) | to_underlying(KeyModifier::RightAlt);
		static constexpr int Meta = to_underlying(KeyModifier::LeftMeta) | to_underlying(KeyModifier::RightMeta);
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
		Key key;
		uint8_t modifiers;
		bool once;
		std::function<void()> fn;
		std::string name;
	};

	typedef std::array<bool, to_underlying(MouseButton::Count)> MouseButtons;

	struct MousePosition
	{
		int32_t x;
		int32_t y;
		int32_t z;

		MousePosition(int32_t px = 0, int32_t py = 0, int32_t pz = 0)
			: x(px)
			, y(py)
			, z(pz)
		{
		}
	};

	struct NormMousePosition
	{
		float x;
		float y;
		float z;

		NormMousePosition(float px = 0.0f, float py = 0.0f, float pz = 0.0f)
			: x(px)
			, y(py)
			, z(pz)
		{
		}
	};

	struct Utf8Char
	{
		uint32_t data;
		uint8_t len;

		Utf8Char(uint32_t pdata = 0, uint8_t plen = 0)
			: data(pdata)
			, len(pdata)
		{
		}
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

	// Keyboard

	///
	void inputSetKeyState(Key key, uint8_t modifiers, bool down);

	///
	bool inputGetKeyState(Key key, uint8_t modifiers = KeyModifiers::None);

	///
	uint8_t inputGetModifiersState();

	/// Adds single UTF-8 encoded character into input buffer.
	void inputPushChar(const Utf8Char& data);

	/// Returns single UTF-8 encoded character from input buffer.
	Utf8Char inputPopChar();

	/// Flush internal input buffer.
	void inputCharFlush();

	///
	char keyToAscii(Key key, uint8_t modifiers);

	///
	const std::string& getKeyName(Key key);

	// Mouse

	///
	void inputSetMouseResolution(const WindowSize& size);

	///
	void inputSetMousePos(const MousePosition& pos);

	///
	void inputSetMouseButtonState(MouseButton button, bool down);

	///
	bool inputGetMouseButtonState(MouseButton button);

	///
	void inputSetMouseLock(bool lock);

	///
	const NormMousePosition& inputPopMouse();

	///
	const MousePosition& inputGetAbsoluteMouse();

	///
	const MouseButtons& inputGetMouseButtons();

	///
	bool inputIsMouseLocked();

	// Gamepad

	///
	void inputSetGamepadAxis(GamepadHandle handle, GamepadAxis axis, int32_t value);

	///
	int32_t inputGetGamepadAxis(GamepadHandle handle, GamepadAxis axis);

}

