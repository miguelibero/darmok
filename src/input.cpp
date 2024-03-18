
#include "input.hpp"
#include <bx/bx.h>
#include "dbg.h"

namespace darmok
{
#pragma region Keyboard

	uint32_t KeyboardImpl::encodeKey(bool down, uint8_t modifiers) noexcept
	{
		uint32_t state = 0;
		state |= uint32_t(down ? modifiers : 0) << 16;
		state |= uint32_t(down) << 8;
		return state;
	}

	bool KeyboardImpl::decodeKey(uint32_t state, uint8_t& modifiers) noexcept
	{
		modifiers = (state >> 16) & 0xff;
		return 0 != ((state >> 8) & 0xff);
	}

	KeyboardImpl::KeyboardImpl() noexcept
		: _keys{}
		, _charsRead(0)
		, _charsWrite(0)
	{
	}

	void KeyboardImpl::reset() noexcept
	{
		_keys.fill(0);
		flush();
	}

	void KeyboardImpl::setKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept
	{
		auto k = to_underlying(key);
		_keys[k] = encodeKey(down, modifiers);
	}

	void KeyboardImpl::pushChar(const Utf8Char& data) noexcept
	{
		_chars[_charsWrite] = data;
		_charsWrite = (_charsWrite + 1) % _chars.size();
	}

	bool KeyboardImpl::getKey(KeyboardKey key) const noexcept
	{
		return _keys[to_underlying(key)];
	}

	bool KeyboardImpl::getKey(KeyboardKey key, uint8_t& modifiers) const noexcept
	{
		return decodeKey(_keys[to_underlying(key)], modifiers);
	}

	const KeyboardKeys& KeyboardImpl::getKeys() const noexcept
	{
		return _keys;
	}

	uint8_t KeyboardImpl::getModifiers() const noexcept
	{
		uint8_t modifiers = 0;
		constexpr auto max = (uint32_t)to_underlying(KeyboardKey::Count);
		for (uint32_t i = 0; i < max; ++i)
		{
			modifiers |= (_keys[i] >> 16) & 0xff;
		}
		return modifiers;
	}

	Utf8Char KeyboardImpl::popChar() noexcept
	{
		if (_charsRead == _charsWrite)
		{
			return {};
		}
		auto v = _chars[_charsRead];
		_charsRead = (_charsRead + 1) % _chars.size();
		return v;
	}

	void KeyboardImpl::flush() noexcept
	{
		_chars.fill({});
		_charsRead = 0;
		_charsWrite = 0;
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

	const std::string& Keyboard::getKeyName(KeyboardKey key) noexcept
	{
		BX_ASSERT(key < KeyboardKey::Count, "Invalid key %d.", key);
		return s_keyboardKeyNames[to_underlying(key)];
	}

	Keyboard::Keyboard() noexcept
		: _impl(std::make_unique<KeyboardImpl>())
	{
	}

	char Keyboard::keyToAscii(KeyboardKey key, uint8_t modifiers) noexcept
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

	bool Keyboard::getKey(KeyboardKey key) const noexcept
	{
		return _impl->getKey(key);
	}

	bool Keyboard::getKey(KeyboardKey key, uint8_t& modifiers) const noexcept
	{
		return _impl->getKey(key, modifiers);
	}

	const KeyboardKeys& Keyboard::getKeys() const noexcept
	{
		return _impl->getKeys();
	}

	uint8_t Keyboard::getModifiers() const noexcept
	{
		return _impl->getModifiers();
	}

	const KeyboardImpl& Keyboard::getImpl() const noexcept
	{
		return *_impl;
	}

	KeyboardImpl& Keyboard::getImpl() noexcept
	{
		return *_impl;
	}

#pragma endregion Keyboard

#pragma region Mouse

	MouseImpl::MouseImpl() noexcept
		: _buttons{}
		, _wheelDelta(120)
		, _lock(false)
		, _absolute{}
		, _relative{}
		, _size{}
	{
	}

	void MouseImpl::setResolution(const glm::uvec2& size) noexcept
	{
		_size = size;
	}

	void MouseImpl::setPosition(const MousePosition& pos) noexcept
	{
		_absolute = pos;
		_relative.x = float(pos.x) / float(_size.x);
		_relative.y = float(pos.y) / float(_size.y);
		_relative.z = float(pos.z) / float(_wheelDelta);
	}

	void MouseImpl::setButton(MouseButton button, bool down) noexcept
	{
		_buttons[to_underlying(button)] = down;
	}

	void MouseImpl::setWheelDelta(uint16_t wheelDelta) noexcept
	{
		_wheelDelta = wheelDelta;
	}

	bool MouseImpl::getButton(MouseButton button) const noexcept
	{
		return _buttons[to_underlying(button)];
	}

	RelativeMousePosition MouseImpl::popRelativePosition() noexcept
	{
		auto rel = _relative;
		_relative = {};
		return rel;
	}

	const MousePosition& MouseImpl::getPosition() const noexcept
	{
		return _absolute;
	}

	const MouseButtons& MouseImpl::getButtons() const noexcept
	{
		return _buttons;
	}

	bool MouseImpl::getLocked() const noexcept
	{
		return _lock;
	}

	bool MouseImpl::setLocked(bool lock) noexcept
	{
		if (_lock != lock)
		{
			_lock = lock;
			_relative = {};
			return true;
		}
		return false;
	}

	static const std::string s_mouseButtonNames[] =
	{
		"Left",
		"Middle",
		"Right",
	};
	BX_STATIC_ASSERT(to_underlying(MouseButton::Count) == BX_COUNTOF(s_mouseButtonNames));

	const std::string& Mouse::getButtonName(MouseButton button) noexcept
	{
		BX_ASSERT(button < MouseButton::Count, "Invalid button %d.", button);
		return s_mouseButtonNames[to_underlying(button)];
	}

	Mouse::Mouse() noexcept
		: _impl(std::make_unique<MouseImpl>())
	{
	}

	bool Mouse::getButton(MouseButton button) const noexcept
	{
		return _impl->getButton(button);
	}

	const MousePosition& Mouse::getPosition() const noexcept
	{
		return _impl->getPosition();
	}

	const MouseButtons& Mouse::getButtons() const noexcept
	{
		return _impl->getButtons();
	}

	bool Mouse::getLocked() const noexcept
	{
		return _impl->getLocked();
	}

	const MouseImpl& Mouse::getImpl() const noexcept
	{
		return *_impl;
	}

	MouseImpl& Mouse::getImpl() noexcept
	{
		return *_impl;
	}

#pragma endregion Mouse

#pragma region Gamepad

	GamepadImpl::GamepadImpl() noexcept
		: _num(Gamepad::MaxAmount)
		, _axes{}
		, _buttons{}
	{
	}

	void GamepadImpl::init(uint8_t num) noexcept
	{
		reset();
		_num = num;
	}

	void GamepadImpl::reset() noexcept
	{
		_axes.fill(0);
		_buttons.fill(false);
		_num = Gamepad::MaxAmount;
	}

	void GamepadImpl::setAxis(GamepadAxis axis, int32_t value) noexcept
	{
		_axes[to_underlying(axis)] = value;
	}

	void GamepadImpl::setButton(GamepadButton button, bool down) noexcept
	{
		_buttons[to_underlying(button)] = down;
	}

	int32_t GamepadImpl::getAxis(GamepadAxis key) const noexcept
	{
		return _axes[to_underlying(key)];
	}

	bool GamepadImpl::getButton(GamepadButton button) const noexcept
	{
		return _buttons[to_underlying(button)];
	}

	const GamepadButtons& GamepadImpl::getButtons() const noexcept
	{
		return _buttons;
	}

	const GamepadAxes& GamepadImpl::getAxes() const noexcept
	{
		return _axes;
	}

	bool GamepadImpl::isConnected() const noexcept
	{
		return _num < Gamepad::MaxAmount;
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

	Gamepad::Gamepad() noexcept
		: _impl(std::make_unique<GamepadImpl>())
	{
	}

	const std::string& Gamepad::getButtonName(GamepadButton button) noexcept
	{
		BX_ASSERT(button < GamepadButton::Count, "Invalid button %d.", button);
		return s_gamepadButtonNames[to_underlying(button)];
	}

	int32_t Gamepad::getAxis(GamepadAxis axis) const noexcept
	{
		return _impl->getAxis(axis);
	}

	bool Gamepad::getButton(GamepadButton button) const noexcept
	{
		return _impl->getButton(button);
	}

	const GamepadAxes& Gamepad::getAxes() const noexcept
	{
		return _impl->getAxes();
	}

	const GamepadButtons& Gamepad::getButtons() const noexcept
	{
		return _impl->getButtons();
	}

	bool Gamepad::isConnected() const noexcept
	{
		return _impl->isConnected();
	}

	const GamepadImpl& Gamepad::getImpl() const noexcept
	{
		return *_impl;
	}

	GamepadImpl& Gamepad::getImpl() noexcept
	{
		return *_impl;
	}

#pragma endregion Gamepad

#pragma region Input

	size_t KeyboardBindingKey::hash() const noexcept
	{
		return to_underlying(key) | modifiers << 16;
	}

	size_t MouseBindingKey::hash() const noexcept
	{
		return to_underlying(button);
	}

	size_t GamepadBindingKey::hash() const noexcept
	{
		return to_underlying(button) << gamepad;
	}

	size_t InputBinding::hashKey(const InputBindingKey& key) noexcept
	{
		if (auto v = std::get_if<KeyboardBindingKey>(&key))
		{
			return to_underlying(v->key) | v->modifiers;
		}

		constexpr size_t maxKey = to_underlying(KeyboardKey::Count) + KeyboardModifiers::Max;

		if (auto v = std::get_if<MouseBindingKey>(&key))
		{
			return maxKey + to_underlying(v->button);
		}
		if (auto v = std::get_if<GamepadBindingKey>(&key))
		{
			auto h = to_underlying(v->button) << v->gamepad;
			return maxKey + to_underlying(MouseButton::Count) + h;
		}
		return 0;
	}

	InputImpl::InputImpl() noexcept
	{
	}

	bool InputImpl::bindingTriggered(InputBinding& binding) noexcept
	{
		if (auto v = std::get_if<KeyboardBindingKey>(&binding.key))
		{
			uint8_t modifiers;
			if (!getKeyboard().getKey(v->key, modifiers))
			{
				return false;
			}
			return modifiers == v->modifiers;
		}
		if (auto v = std::get_if<MouseBindingKey>(&binding.key))
		{
			return getMouse().getButton(v->button);
		}
		if (auto v = std::get_if<GamepadBindingKey>(&binding.key))
		{
			auto gamepad = getGamepad(v->gamepad);
			if (gamepad.hasValue())
			{
				return gamepad->getButton(v->button);
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

	void InputImpl::processBinding(InputBinding& binding) noexcept
	{
		bool triggered = bindingTriggered(binding);
		if (binding.once)
		{
			if (triggered)
			{
				auto itr = _bindingOnce.find(binding.key);
				if (itr == _bindingOnce.end())
				{
					if (binding.fn != nullptr)
					{
						binding.fn();
					}
					_bindingOnce.emplace(binding.key);
				}
			}
			else
			{
				_bindingOnce.erase(binding.key);
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

	void InputImpl::process() noexcept
	{
		for (auto& elm : _bindings)
		{
			for (auto& binding : elm.second)
			{
				processBinding(binding);
			}
		}
	}

	void InputImpl::reset() noexcept
	{
		_bindingOnce.clear();
	}

	void InputImpl::addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept
	{
		std::string nameStr(name);
		auto it = _bindings.find(nameStr);
		_bindings.insert(it, std::make_pair(nameStr, std::move(bindings)));
	}

	void InputImpl::removeBindings(std::string_view name) noexcept
	{
		std::string nameStr(name);
		auto it = _bindings.find(nameStr);
		if (it != _bindings.end())
		{
			_bindings.erase(it);
		}
	}

	Keyboard& InputImpl::getKeyboard() noexcept
	{
		return _keyboard;
	}

	Mouse& InputImpl::getMouse() noexcept
	{
		return _mouse;
	}

	OptionalRef<Gamepad> InputImpl::getGamepad(uint8_t num) noexcept
	{
		if (num < Gamepad::MaxAmount)
		{
			return _gamepads[num];
		}
		return nullptr;
	}

	Gamepads& InputImpl::getGamepads() noexcept
	{
		return _gamepads;
	}

	const InputState& InputImpl::getState() const noexcept
	{
		return _state;
	}

	void InputImpl::update() noexcept
	{
		_state.mouse = _mouse.getImpl().popRelativePosition();
		auto& kb = _keyboard.getImpl();
		_state.chars.clear();
		Utf8Char c;
		while(true)
		{
			c = kb.popChar();
			if (c.len == 0)
			{
				break;
			}
			_state.chars.push_back(c);
		}
	}

	Input::Input() noexcept
		: _impl(std::make_unique<InputImpl>())
	{
	}

	void Input::addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept
	{
		_impl->addBindings(name, std::move(bindings));
	}

	void Input::removeBindings(std::string_view name) noexcept
	{
		_impl->removeBindings(name);
	}

	void Input::process() noexcept
	{
		_impl->process();
	}

	Keyboard& Input::getKeyboard() noexcept
	{
		return _impl->getKeyboard();
	}

	const Keyboard& Input::getKeyboard() const noexcept
	{
		return _impl->getKeyboard();
	}

	Mouse& Input::getMouse() noexcept
	{
		return _impl->getMouse();
	}

	const Mouse& Input::getMouse() const noexcept
	{
		return _impl->getMouse();
	}

	OptionalRef<Gamepad> Input::getGamepad(uint8_t num) noexcept
	{
		return _impl->getGamepad(num);
	}

	OptionalRef<const Gamepad> Input::getGamepad(uint8_t num) const noexcept
	{
		auto gamepad = _impl->getGamepad(num);
		if (gamepad)
		{
			return gamepad.value();
		}
		return nullptr;
	}

	Gamepads& Input::getGamepads() noexcept
	{
		return _impl->getGamepads();
	}

	const Gamepads& Input::getGamepads() const noexcept
	{
		return _impl->getGamepads();
	}

	const InputImpl& Input::getImpl() const noexcept
	{
		return *_impl;
	}

	InputImpl& Input::getImpl() noexcept
	{
		return *_impl;
	}

#pragma endregion Input

}