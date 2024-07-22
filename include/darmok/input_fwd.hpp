#pragma once

#include <darmok/utils.hpp>

namespace darmok
{
    enum class KeyboardModifier
	{
		Alt,
		Ctrl,
		Shift,
		Meta,

		Count
	};

	enum class KeyboardKey
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
		GraveAccent,
		CapsLock,
		NumLock,
		ScrollLock,
		Pause,
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

		Count
	};

	enum class InputAxisType
	{
		Horizontal,
		Vertical,

		Count
	};

	enum class MouseInputType
	{
		Position,
		Scroll,
		
		Count
	};

    enum class MouseButton
	{
		Left,
		Middle,
		Right,

		Count
	};

    enum class GamepadStick
	{
		Left,
		Right,
		Count
	};

	enum class GamepadButton
	{
		None = 0,
		
		A,
		B,
		X,
		Y,
		ThumbL,
		ThumbR,
		ShoulderL,
		ShoulderR,
		Up,
		Down,
		Left,
		Right,
		Back,
		Start,
		Guide,

		Count
	};
}