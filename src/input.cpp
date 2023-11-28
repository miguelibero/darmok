
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/utils.hpp>
#include <bx/bx.h>

#include <string>
#include <unordered_map>
#include <array>

namespace darmok
{
	static const std::string s_keyNames[] =
	{
		"None",
		"Esc",
		"Return",
		"Tab",
		"Space",
		"Backspace",
		"Up",
		"Down",
		"Left",
		"Right",
		"Insert",
		"Delete",
		"Home",
		"End",
		"PageUp",
		"PageDown",
		"Print",
		"Plus",
		"Minus",
		"LeftBracket",
		"RightBracket",
		"Semicolon",
		"Quote",
		"Comma",
		"Period",
		"Slash",
		"Backslash",
		"Tilde",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"NumPad0",
		"NumPad1",
		"NumPad2",
		"NumPad3",
		"NumPad4",
		"NumPad5",
		"NumPad6",
		"NumPad7",
		"NumPad8",
		"NumPad9",
		"Key0",
		"Key1",
		"Key2",
		"Key3",
		"Key4",
		"Key5",
		"Key6",
		"Key7",
		"Key8",
		"Key9",
		"KeyA",
		"KeyB",
		"KeyC",
		"KeyD",
		"KeyE",
		"KeyF",
		"KeyG",
		"KeyH",
		"KeyI",
		"KeyJ",
		"KeyK",
		"KeyL",
		"KeyM",
		"KeyN",
		"KeyO",
		"KeyP",
		"KeyQ",
		"KeyR",
		"KeyS",
		"KeyT",
		"KeyU",
		"KeyV",
		"KeyW",
		"KeyX",
		"KeyY",
		"KeyZ",
		"GamepadA",
		"GamepadB",
		"GamepadX",
		"GamepadY",
		"GamepadThumbL",
		"GamepadThumbR",
		"GamepadShoulderL",
		"GamepadShoulderR",
		"GamepadUp",
		"GamepadDown",
		"GamepadLeft",
		"GamepadRight",
		"GamepadBack",
		"GamepadStart",
		"GamepadGuide",
	};
	BX_STATIC_ASSERT(to_underlying(Key::Count) == BX_COUNTOF(s_keyNames));

	const std::string& getKeyName(Key key)
	{
		BX_ASSERT(key < Key::Count, "Invalid key %d.", key);
		return s_keyNames[to_underlying(key)];
	}

	char keyToAscii(Key key, uint8_t modifiers)
	{
		const bool isAscii = (Key::Key0 <= key && key <= Key::KeyZ)
						  || (Key::Esc  <= key && key <= Key::Minus);
		if (!isAscii)
		{
			return '\0';
		}

		const bool isNumber = (Key::Key0 <= key && key <= Key::Key9);
		if (isNumber)
		{
			return '0' + char(to_underlying(key) - to_underlying(Key::Key0));
		}

		const bool isChar = (Key::KeyA <= key && key <= Key::KeyZ);
		if (isChar)
		{
			enum { ShiftMask = to_underlying(KeyModifier::LeftShift) | to_underlying(KeyModifier::RightShift) };

			const bool shift = !!( modifiers & ShiftMask );
			return (shift ? 'A' : 'a') + char(to_underlying(key) - to_underlying(Key::KeyA));
		}

		switch (key)
		{
		case Key::Esc:       return 0x1b;
		case Key::Return:    return '\n';
		case Key::Tab:       return '\t';
		case Key::Space:     return ' ';
		case Key::Backspace: return 0x08;
		case Key::Plus:      return '+';
		case Key::Minus:     return '-';
		default:             break;
		}

		return '\0';
	}

	class InputMouse final
	{
	private:
		NormMousePosition kDefaultNorm = { 0.0f, 0.0f, 0.0f };

	public:

		InputMouse()
			: _buttons{}
			, _size{ 1280, 720 }
			, _wheelDelta(120)
			, _lock(false)
		{
		}

		void resetNorm()
		{
			_norm = kDefaultNorm;
		}

		void setResolution(const WindowSize& size)
		{
			_size = size;
		}

		void setPos(const MousePosition& pos)
		{
			_absolute = pos;
			_norm.x = float(pos.x) / float(_size.width);
			_norm.y = float(pos.y) / float(_size.height);
			_norm.z = float(pos.z) / float(_wheelDelta);
		}

		void setButtonState(MouseButton button, bool down)
		{
			_buttons[to_underlying(button)] = down;
		}

		bool getButtonState(MouseButton button) const
		{
			return _buttons[to_underlying(button)];
		}

		const NormMousePosition& getNorm() const
		{
			return _norm;
		}

		NormMousePosition popNorm()
		{
			auto norm = _norm;
			resetNorm();
			return norm;
		}

		const MousePosition& getAbs() const
		{
			return _absolute;
		}

		const MouseButtons& getButtonStates() const
		{
			return _buttons;
		}

		bool isLocked() const
		{
			return _lock;
		}

		bool setLock(bool lock)
		{
			if (_lock != lock)
			{
				_lock = lock;
				resetNorm();
				return true;
			}
			return false;
		}

	private:
		MousePosition _absolute;
		NormMousePosition _norm;

		MouseButtons _buttons;
		WindowSize _size;
		uint16_t _wheelDelta;
		bool _lock;
	};

	class InputKeyboard final
	{
	public:

		InputKeyboard()
			: _charsRead(0)
			, _charsWrite(0)
		{
			_keys.fill(0);
			_once.fill(false);
		}
		void reset()
		{
			_keys.fill(0);
			_once.fill(false);
			charFlush();
		}

		void setKeyState(Key key, uint8_t modifiers, bool down)
		{
			auto k = to_underlying(key);
			_keys[k] = encodeKeyState(modifiers, down);
			_once[k] = false;
		}

		bool getKeyState(Key key, uint8_t& modifiers)
		{
			return decodeKeyState(_keys[to_underlying(key)], modifiers);
		}

		bool getKeyState(Key key)
		{
			uint8_t modifiers;
			return getKeyState(key, modifiers);
		}

		uint8_t getModifiersState()
		{
			uint8_t modifiers = 0;
			constexpr auto max = (uint32_t)to_underlying(Key::Count);
			for (uint32_t i = 0; i < max; ++i)
			{
				modifiers |= (_keys[i] >> 16) & 0xff;
			}
			return modifiers;
		}

		void pushChar(const Utf8Char& v)
		{
			_chars[_charsWrite] = v;
			_charsWrite = (_charsWrite + 1) % _chars.size();
		}

		Utf8Char popChar()
		{
			if (_charsRead == _charsWrite)
			{
				return {};
			}
			auto v = _chars[_charsRead];
			_charsRead = (_charsRead + 1) % _chars.size();
			return v;
		}

		static uint32_t encodeKeyState(uint8_t modifiers, bool down)
		{
			uint32_t state = 0;
			state |= uint32_t(down ? modifiers : 0) << 16;
			state |= uint32_t(down) << 8;
			return state;
		}

		static bool decodeKeyState(uint32_t state, uint8_t& modifiers)
		{
			modifiers = (state >> 16) & 0xff;
			return 0 != ((state >> 8) & 0xff);
		}

		void charFlush()
		{
			_chars.fill({});
			_charsRead = 0;
			_charsWrite = 0;
		}

		void processBinding(const InputBinding& binding)
		{
			uint8_t modifiers;
			auto k = to_underlying(binding.key);
			bool down = decodeKeyState(_keys[k], modifiers);

			if (binding.once)
			{
				if (down)
				{
					if (modifiers == binding.modifiers
						&& !_once[k])
					{
						if (binding.fn != nullptr)
						{
							binding.fn();
						}
						_once[k] = true;
					}
				}
				else
				{
					_once[k] = false;
				}
			}
			else
			{
				if (down
					&& modifiers == binding.modifiers)
				{
					if (binding.fn != nullptr)
					{
						binding.fn();
					}
				}
			}
		}

	private:

		std::array<uint32_t, 256> _keys;
		std::array<bool, 256> _once;
		std::array<Utf8Char, 256> _chars;
	
		size_t _charsRead;
		size_t _charsWrite;
	};

	class InputGamepad final
	{
	public:
		InputGamepad()
		{
			reset();
		}

		void reset()
		{
			_axis.fill(0);
		}

		void setAxis(GamepadAxis key, int32_t value)
		{
			_axis[to_underlying(key)] = value;
		}

		int32_t getAxis(GamepadAxis key)
		{
			return _axis[to_underlying(key)];
		}

	private:
		std::array<int32_t, to_underlying(GamepadAxis::Count)> _axis;
	};

	class Input final
	{
	public:
		Input()
		{
			reset();
		}

		~Input()
		{
		}

		void addBindings(const std::string& name, std::vector<InputBinding>&& value)
		{
			auto it = _bindings.find(name);
			_bindings.insert(it, std::make_pair(name, std::move(value)));
		}

		void removeBindings(const std::string& name)
		{
			auto it = _bindings.find(name);
			if (it != _bindings.end())
			{
				_bindings.erase(it);
			}
		}

		void process()
		{
			for(auto& elm : _bindings)
			{
				for (auto& binding : elm.second)
				{
					_keyboard.processBinding(binding);
				}
			}
		}

		void reset()
		{
			_mouse.resetNorm();
			_keyboard.reset();
			for(auto& gamepad : _gamepads)
			{
				gamepad.reset();
			}
		}

		InputKeyboard& getKeyboard()
		{
			return _keyboard;
		}

		const InputKeyboard& getKeyboard() const
		{
			return _keyboard;
		}

		InputMouse& getMouse()
		{
			return _mouse;
		}

		const InputMouse& getMouse() const
		{
			return _mouse;
		}

		InputGamepad& getGamepad(size_t k)
		{
			return _gamepads[k];
		}

		const InputGamepad& getGamepad(size_t k) const
		{
			return _gamepads[k];
		}

	private:
		typedef std::unordered_map<std::string, std::vector<InputBinding>> BindingMap;
		BindingMap _bindings;
		InputKeyboard _keyboard;
		InputMouse _mouse;
		std::array<InputGamepad, DARMOK_CONFIG_MAX_GAMEPADS> _gamepads;
	};

	static Input input;

	void inputInit()
	{
	}

	void inputShutdown()
	{
	}

	void inputAddBindings(const std::string& name, std::vector<InputBinding>&& bindings)
	{
		input.addBindings(name, std::move(bindings));
	}

	void inputRemoveBindings(const std::string& name)
	{
		input.removeBindings(name);
	}

	void inputProcess()
	{
		input.process();
	}

	void inputSetMouseResolution(const WindowSize& size)
	{
		input.getMouse().setResolution(size);
	}

	void inputSetKeyState(Key key, uint8_t modifiers, bool down)
	{
		input.getKeyboard().setKeyState(key, modifiers, down);
	}

	bool inputGetKeyState(Key key, uint8_t modifiers)
	{
		return input.getKeyboard().getKeyState(key, modifiers);
	}

	uint8_t inputGetModifiersState()
	{
		return input.getKeyboard().getModifiersState();
	}

	void inputPushChar(const Utf8Char& data)
	{
		input.getKeyboard().pushChar(data);
	}

	Utf8Char inputPopChar()
	{
		return input.getKeyboard().popChar();
	}

	void inputCharFlush()
	{
		input.getKeyboard().charFlush();
	}

	void inputSetMousePos(const MousePosition& v)
	{
		input.getMouse().setPos(v);
	}

	void inputSetMouseButtonState(MouseButton button, bool down)
	{
		input.getMouse().setButtonState(button, down);
	}

	bool inputGetMouseButtonState(MouseButton button)
	{
		return input.getMouse().getButtonState(button);
	}

	const MousePosition& inputGetAbsoluteMouse()
	{
		return input.getMouse().getAbs();
	}

	const NormMousePosition& inputPopMouse()
	{
		return input.getMouse().popNorm();
	}

	const MouseButtons& inputGetMouseButtons()
	{
		return input.getMouse().getButtonStates();
	}

	bool inputIsMouseLocked()
	{
		return input.getMouse().isLocked();
	}

	void inputSetMouseLock(bool lock)
	{
		input.getMouse().setLock(lock);
	}

	void inputSetGamepadAxis(GamepadHandle handle, GamepadAxis axis, int32_t value)
	{
		input.getGamepad(handle.idx).setAxis(axis, value);
	}

	int32_t inputGetGamepadAxis(GamepadHandle handle, GamepadAxis axis)
	{
		return input.getGamepad(handle.idx).getAxis(axis);
	}
}