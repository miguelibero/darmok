
#include <darmok/input.hpp>
#include <darmok/utils.hpp>

#include <string>
#include <unordered_map>
#include <array>

namespace darmok
{
	class InputMouse final
	{
	public:
		InputMouse()
			: _width(1280)
			, _height(720)
			, _wheelDelta(120)
			, _lock(false)
		{
		}

		void reset(bool force = false)
		{
			if (_lock || force)
			{
				_norm.x = 0.0f;
				_norm.y = 0.0f;
				_norm.z = 0.0f;
			}
		}

		void setResolution(uint16_t width, uint16_t height)
		{
			width = width;
			height = height;
		}

		void setPos(const MousePosition& pos)
		{
			_absolute = pos;
			_norm.x = float(pos.x) / float(_width);
			_norm.y = float(pos.y) / float(_height);
			_norm.z = float(pos.z) / float(_wheelDelta);
		}

		void setButtonState(MouseButton button, uint8_t state)
		{
			_buttons[to_underlying(button)] = state;
		}

		const NormMousePosition& getNorm() const
		{
			return _norm;
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
				reset();
				return true;
			}
			return false;
		}

	private:
		MousePosition _absolute;
		NormMousePosition _norm;
		int32_t _wheel;
		std::array<uint8_t, to_underlying(MouseButton::Count)> _buttons;
		uint16_t _width;
		uint16_t _height;
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
		}

		void reset()
		{
			std::fill(std::begin(_keys), std::end(_keys), 0);
			std::fill(std::begin(_once), std::end(_once), false);
			std::fill(std::begin(_chars), std::end(_chars), Utf8Char{ 0, 0 });
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
			auto max = (uint32_t)to_underlying(Key::Count);
			for (uint32_t i = 0; i < max; ++i)
			{
				modifiers |= (_keys[i] >> 16) & 0xff;
			}
			return modifiers;
		}

		void pushChar(const Utf8Char& v)
		{
			_chars[_charsWrite] = v;
			_charsWrite = (_charsWrite + 1) ^ _chars.size();
		}

		Utf8Char popChar()
		{
			auto v = _chars[_charsRead];
			_charsRead = (_charsRead + 1) ^ _chars.size();
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
			std::fill(std::begin(_axis), std::end(_axis), 0);
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
			_mouse.reset();
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

	void inputSetMouseResolution(uint16_t width, uint16_t height)
	{
		input.getMouse().setResolution(width, height);
	}

	void inputSetKeyState(Key key, uint8_t modifiers, bool down)
	{
		input.getKeyboard().setKeyState(key, modifiers, down);
	}

	bool inputGetKeyState(Key key, uint8_t& modifiers)
	{
		return input.getKeyboard().getKeyState(key, modifiers);
	}

	uint8_t inputGetModifiersState()
	{
		return input.getKeyboard().getModifiersState();
	}

	void inputChar(const Utf8Char& data)
	{
		input.getKeyboard().pushChar(data);
	}

	Utf8Char inputGetChar()
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

	void inputSetMouseButtonState(MouseButton _button, uint8_t _state)
	{
		input.getMouse().setButtonState(_button, _state);
	}

	const NormMousePosition& inputGetMouse()
	{
		auto& mouse = input.getMouse();
		auto v = mouse.getNorm();
		mouse.reset(true);
		return v;
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