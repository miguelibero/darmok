
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/utils.hpp>
#include <bx/bx.h>

#include <string>
#include <unordered_map>
#include <array>

#include "input.hpp"

namespace darmok
{
#pragma region Keyboard

	Utf8Char::Utf8Char(uint32_t pdata, uint8_t plen)
		: data(pdata)
		, len(pdata)
	{
	}

	uint32_t KeyboardImpl::encodeKey(bool down, uint8_t modifiers)
	{
		uint32_t state = 0;
		state |= uint32_t(down ? modifiers : 0) << 16;
		state |= uint32_t(down) << 8;
		return state;
	}

	bool KeyboardImpl::decodeKey(uint32_t state, uint8_t& modifiers)
	{
		modifiers = (state >> 16) & 0xff;
		return 0 != ((state >> 8) & 0xff);
	}

	KeyboardImpl::KeyboardImpl()
		: _keys{}
		, _charsRead(0)
		, _charsWrite(0)
	{
	}

	void KeyboardImpl::reset()
	{
		_keys.fill(0);
		flush();
	}

	void KeyboardImpl::setKey(KeyboardKey key, uint8_t modifiers, bool down)
	{
		auto k = to_underlying(key);
		_keys[k] = encodeKey(down, modifiers);
	}

	void KeyboardImpl::pushChar(const Utf8Char& data)
	{
		_chars[_charsWrite] = data;
		_charsWrite = (_charsWrite + 1) % _chars.size();
	}

	bool KeyboardImpl::getKey(KeyboardKey key) const
	{
		return _keys[to_underlying(key)];
	}

	bool KeyboardImpl::getKey(KeyboardKey key, uint8_t& modifiers) const
	{
		return decodeKey(_keys[to_underlying(key)], modifiers);
	}

	const KeyboardKeys& KeyboardImpl::getKeys() const
	{
		return _keys;
	}

	uint8_t KeyboardImpl::getModifiers() const
	{
		uint8_t modifiers = 0;
		constexpr auto max = (uint32_t)to_underlying(KeyboardKey::Count);
		for (uint32_t i = 0; i < max; ++i)
		{
			modifiers |= (_keys[i] >> 16) & 0xff;
		}
		return modifiers;
	}

	Utf8Char KeyboardImpl::popChar()
	{
		if (_charsRead == _charsWrite)
		{
			return {};
		}
		auto v = _chars[_charsRead];
		_charsRead = (_charsRead + 1) % _chars.size();
		return v;
	}

	void KeyboardImpl::flush()
	{
		_chars.fill({});
		_charsRead = 0;
		_charsWrite = 0;
	}

	KeyboardImpl& KeyboardImpl::get()
	{
		static KeyboardImpl instance;
		return instance;
	}

	static const std::string s_keyboardKeyNames[] =
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
	};
	BX_STATIC_ASSERT(to_underlying(KeyboardKey::Count) == BX_COUNTOF(s_keyboardKeyNames));

	const std::string& Keyboard::getKeyName(KeyboardKey key)
	{
		BX_ASSERT(key < KeyboardKey::Count, "Invalid key %d.", key);
		return s_keyboardKeyNames[to_underlying(key)];
	}

	char Keyboard::keyToAscii(KeyboardKey key, uint8_t modifiers)
	{
		const bool isAscii = (KeyboardKey::Key0 <= key && key <= KeyboardKey::KeyZ)
						  || (KeyboardKey::Esc  <= key && key <= KeyboardKey::Minus);
		if (!isAscii)
		{
			return '\0';
		}

		const bool isNumber = (KeyboardKey::Key0 <= key && key <= KeyboardKey::Key9);
		if (isNumber)
		{
			return '0' + char(to_underlying(key) - to_underlying(KeyboardKey::Key0));
		}

		const bool isChar = (KeyboardKey::KeyA <= key && key <= KeyboardKey::KeyZ);
		if (isChar)
		{
			const bool shift = !!( modifiers & KeyboardModifiers::Shift );
			return (shift ? 'A' : 'a') + char(to_underlying(key) - to_underlying(KeyboardKey::KeyA));
		}

		switch (key)
		{
		case KeyboardKey::Esc:       return 0x1b;
		case KeyboardKey::Return:    return '\n';
		case KeyboardKey::Tab:       return '\t';
		case KeyboardKey::Space:     return ' ';
		case KeyboardKey::Backspace: return 0x08;
		case KeyboardKey::Plus:      return '+';
		case KeyboardKey::Minus:     return '-';
		default:             break;
		}

		return '\0';
	}

	bool Keyboard::getKey(KeyboardKey key) const
	{
		return KeyboardImpl::get().getKey(key);
	}

	bool Keyboard::getKey(KeyboardKey key, uint8_t& modifiers) const
	{
		return KeyboardImpl::get().getKey(key, modifiers);
	}

	const KeyboardKeys& Keyboard::getKeys() const
	{
		return KeyboardImpl::get().getKeys();
	}

	uint8_t Keyboard::getModifiers() const
	{
		return KeyboardImpl::get().getModifiers();
	}

	Utf8Char Keyboard::popChar()
	{
		return KeyboardImpl::get().popChar();
	}

	void Keyboard::flush()
	{
		return KeyboardImpl::get().flush();
	}

#pragma endregion Keyboard

#pragma region Mouse

	MousePosition::MousePosition(int32_t px, int32_t py, int32_t pz)
		: x(px)
		, y(py)
		, z(pz)
	{
	}

	RelativeMousePosition::RelativeMousePosition(float px, float py, float pz)
		: x(px)
		, y(py)
		, z(pz)
	{
	}

	void MouseImpl::setResolution(const WindowSize& size)
	{
		_size = size;
	}

	void MouseImpl::setPosition(const MousePosition& pos)
	{
		_absolute = pos;
		_relative.x = float(pos.x) / float(_size.width);
		_relative.y = float(pos.y) / float(_size.height);
		_relative.z = float(pos.z) / float(_wheelDelta);
	}

	void MouseImpl::setButton(MouseButton button, bool down)
	{
		_buttons[to_underlying(button)] = down;
	}

	MouseImpl::MouseImpl()
		: _buttons{}
		, _wheelDelta(120)
		, _lock(false)
	{
	}

	void MouseImpl::setWheelDelta(uint16_t wheelDelta)
	{
		_wheelDelta = wheelDelta;
	}

	bool MouseImpl::getButton(MouseButton button) const
	{
		return _buttons[to_underlying(button)];
	}

	RelativeMousePosition MouseImpl::popRelativePosition()
	{
		auto rel = _relative;
		_relative = {};
		return rel;
	}

	const MousePosition& MouseImpl::getPosition() const
	{
		return _absolute;
	}

	const MouseButtons& MouseImpl::getButtons() const
	{
		return _buttons;
	}

	bool MouseImpl::getLocked() const
	{
		return _lock;
	}

	bool MouseImpl::setLocked(bool lock)
	{
		if (_lock != lock)
		{
			_lock = lock;
			_relative = {};
			return true;
		}
		return false;
	}

	MouseImpl& MouseImpl::get()
	{
		static MouseImpl instance;
		return instance;
	}

	static const std::string s_mouseButtonNames[] =
	{
		"None",
		"Left",
		"Middle",
		"Right",
	};
	BX_STATIC_ASSERT(to_underlying(MouseButton::Count) == BX_COUNTOF(s_mouseButtonNames));

	const std::string& Mouse::getButtonName(MouseButton button)
	{
		BX_ASSERT(button < MouseButton::Count, "Invalid button %d.", button);
		return s_mouseButtonNames[to_underlying(button)];
	}

	void Mouse::setWheelDelta(float wheelDelta)
	{
		MouseImpl::get().setWheelDelta(wheelDelta);
	}

	bool Mouse::getButton(MouseButton button) const
	{
		return MouseImpl::get().getButton(button);
	}

	RelativeMousePosition Mouse::popRelativePosition()
	{
		return MouseImpl::get().popRelativePosition();
	}

	const MousePosition& Mouse::getPosition() const
	{
		return MouseImpl::get().getPosition();
	}

	const MouseButtons& Mouse::getButtons() const
	{
		return MouseImpl::get().getButtons();
	}

	bool Mouse::getLocked() const
	{
		return MouseImpl::get().getLocked();
	}

	bool Mouse::setLocked(bool lock)
	{
		return MouseImpl::get().setLocked(lock);
	}

#pragma endregion Mouse

#pragma region Gamepad

	bool GamepadHandle::operator==(const GamepadHandle& other) const
	{
		return idx == other.idx;
	}

	bool GamepadHandle::operator<(const GamepadHandle& other) const
	{
		return idx < other.idx;
	}

	bool GamepadHandle::isValid() const
	{
		return idx < Gamepad::MaxAmount;
	}

	GamepadImpl::GamepadImpl()
		: _connected(false)
		, _axes{}
		, _buttons{}
	{
	}

	void GamepadImpl::reset()
	{
		_axes.fill(0);
		_buttons.fill(false);
		_connected = false;
	}

	void GamepadImpl::setAxis(GamepadAxis axis, int32_t value)
	{
		_axes[to_underlying(axis)] = value;
	}

	void GamepadImpl::setButton(GamepadButton button, bool down)
	{
		_buttons[to_underlying(button)] = down;
	}

	void GamepadImpl::setConnected(bool connected)
	{
		_connected = connected;
	}

	int32_t GamepadImpl::getAxis(GamepadAxis key) const
	{
		return _axes[to_underlying(key)];
	}

	bool GamepadImpl::getButton(GamepadButton button) const
	{
		return _buttons[to_underlying(button)];
	}

	const GamepadButtons& GamepadImpl::getButtons() const
	{
		return _buttons;
	}

	const GamepadAxes& GamepadImpl::getAxes() const
	{
		return _axes;
	}

	bool GamepadImpl::getConnected() const
	{
		return _connected;
	}

	GamepadImpl& GamepadImpl::get(const GamepadHandle& handle)
	{
		static Container instances;
		return instances[handle.idx];
	}

	static const std::string s_gamepadButtonNames[] =
	{
		"None",
		"A",
		"B",
		"X",
		"Y",
		"ThumbL",
		"ThumbR",
		"ShoulderL",
		"ShoulderR",
		"Up",
		"Down",
		"Left",
		"Right",
		"Back",
		"Start",
		"Guide",
	};
	BX_STATIC_ASSERT(to_underlying(GamepadButton::Count) == BX_COUNTOF(s_gamepadButtonNames));

	Gamepad::Gamepad()
		: _handle(Gamepad::InvalidHandle)
	{
	}

	void Gamepad::setHandle(const GamepadHandle& handle)
	{
		_handle = handle;
	}

	const std::string& Gamepad::getButtonName(GamepadButton button)
	{
		BX_ASSERT(button < GamepadButton::Count, "Invalid button %d.", button);
		return s_gamepadButtonNames[to_underlying(button)];
	}

	int32_t Gamepad::getAxis(GamepadAxis key) const
	{
		return GamepadImpl::get(_handle).getAxis(key);
	}

	bool Gamepad::getButton(GamepadButton button) const
	{
		return GamepadImpl::get(_handle).getButton(button);
	}

	const GamepadButtons& Gamepad::getButtons() const
	{
		return GamepadImpl::get(_handle).getButtons();
	}

	bool Gamepad::getConnected() const
	{
		return GamepadImpl::get(_handle).getConnected();
	}

#pragma endregion Gamepad

#pragma region Input

	size_t InputBinding::hashKey(const InputBindingKey& key)
	{
		if (auto v = std::get_if<KeyboardInputBinding>(&key))
		{
			return to_underlying(v->key) | v->modifiers;
		}
		if (auto v = std::get_if<MouseInputBinding>(&key))
		{
			return KeyboardModifiers::Max + to_underlying(v->button);
		}
		if (auto v = std::get_if<GamepadInputBinding>(&key))
		{
			auto h = to_underlying(v->button) >> v->gamepad.idx;
			return KeyboardModifiers::Max + to_underlying(MouseButton::Count) + h;
		}
		return 0;
	}

	InputImpl::InputImpl()
		: _gamepads{}
	{
		GamepadHandle handle{ 0 };
		for (auto& gamepad : getGamepads())
		{
			gamepad.setHandle(handle);
			handle.idx++;
		}
	}

	bool InputImpl::bindingTriggered(InputBinding& binding)
	{
		if (auto v = std::get_if<KeyboardInputBinding>(&binding.key))
		{
			uint8_t modifiers;
			if (!getKeyboard().getKey(v->key, modifiers))
			{
				return false;
			}
			return modifiers == v->modifiers;
		}
		if (auto v = std::get_if<MouseInputBinding>(&binding.key))
		{
			return getMouse().getButton(v->button);
		}
		if (auto v = std::get_if<GamepadInputBinding>(&binding.key))
		{
			if (v->gamepad.isValid())
			{
				return getGamepad(v->gamepad).getButton(v->button);
			}
			else
			{
				for (auto& gamepad : getGamepads())
				{
					if (gamepad.getButton(v->button))
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	void InputImpl::processBinding(InputBinding& binding)
	{
		bool triggered = bindingTriggered(binding);
		if (binding.once)
		{
			auto keyHash = InputBinding::hashKey(binding.key);
			if (triggered)
			{
				auto itr = _bindingOnce.find(keyHash);
				if (itr == _bindingOnce.end() || itr->second == false)
				{
					if (binding.fn != nullptr)
					{
						binding.fn();
					}
					_bindingOnce[keyHash] = true;
				}
			}
			else
			{
				_bindingOnce[keyHash] = false;
			}
		}
		else if (triggered)
		{
			if (binding.fn != nullptr)
			{
				binding.fn();
			}
		}
	}

	void InputImpl::process()
	{
		for (auto& elm : _bindings)
		{
			for (auto& binding : elm.second)
			{
				processBinding(binding);
			}
		}
	}

	void InputImpl::reset()
	{
		_bindingOnce.clear();
	}

	void InputImpl::addBindings(const std::string& name, std::vector<InputBinding>&& bindings)
	{
		auto it = _bindings.find(name);
		_bindings.insert(it, std::make_pair(name, std::move(bindings)));
	}

	void InputImpl::removeBindings(const std::string& name)
	{
		auto it = _bindings.find(name);
		if (it != _bindings.end())
		{
			_bindings.erase(it);
		}
	}

	Keyboard& InputImpl::getKeyboard()
	{
		return _keyboard;
	}

	Mouse& InputImpl::getMouse()
	{
		return _mouse;
	}

	Gamepad& InputImpl::getGamepad(const GamepadHandle& handle)
	{
		return _gamepads[handle.idx];
	}

	Gamepads& InputImpl::getGamepads()
	{
		return _gamepads;
	}

	InputImpl& InputImpl::get()
	{
		static InputImpl instance;
		return instance;
	}

	void Input::addBindings(const std::string& name, std::vector<InputBinding>&& value)
	{
		InputImpl::get().addBindings(name, std::move(value));
	}

	void Input::removeBindings(const std::string& name)
	{
		InputImpl::get().removeBindings(name);
	}

	void Input::process()
	{
		InputImpl::get().process();
	}

	Keyboard& Input::getKeyboard()
	{
		return InputImpl::get().getKeyboard();
	}

	const Keyboard& Input::getKeyboard() const
	{
		return InputImpl::get().getKeyboard();
	}

	Mouse& Input::getMouse()
	{
		return InputImpl::get().getMouse();
	}

	const Mouse& Input::getMouse() const
	{
		return InputImpl::get().getMouse();
	}

	Gamepad& Input::getGamepad(const GamepadHandle& handle)
	{
		return InputImpl::get().getGamepad(handle);
	}

	const Gamepad& Input::getGamepad(const GamepadHandle& handle) const
	{
		return InputImpl::get().getGamepad(handle);
	}

	Gamepads& Input::getGamepads()
	{
		return InputImpl::get().getGamepads();
	}

	const Gamepads& Input::getGamepads() const
	{
		return InputImpl::get().getGamepads();
	}

	Input& Input::get()
	{
		static Input instance;
		return instance;
	}

#pragma endregion Input

}