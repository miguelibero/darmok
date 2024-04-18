
#include "input.hpp"
#include <bx/bx.h>
#include <darmok/window.hpp>
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
		if (k < _keys.size())
		{
			_keys[k] = encodeKey(down, modifiers);
		}
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

	const KeyboardChars& KeyboardImpl::getUpdateChars() const noexcept
	{
		return _updateChars;
	}

	void KeyboardImpl::update() noexcept
	{
		_updateChars.clear();
		Utf8Char c;
		while (true)
		{
			c = popChar();
			if (c.len == 0)
			{
				break;
			}
			_updateChars.push_back(c);
		}
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

	const KeyboardChars& Keyboard::getUpdateChars() const noexcept
	{
		return _impl->getUpdateChars();
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
		, _position{}
	{
	}

	void MouseImpl::setPosition(const glm::vec3& pos) noexcept
	{
		_position = pos;
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


	const glm::vec3& MouseImpl::getPosition() const noexcept
	{
		return _position;
	}

	const MouseButtons& MouseImpl::getButtons() const noexcept
	{
		return _buttons;
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

	const glm::vec3& Mouse::getPosition() const noexcept
	{
		return _impl->getPosition();
	}

	const MouseButtons& Mouse::getButtons() const noexcept
	{
		return _impl->getButtons();
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
		, _sticks{}
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
		_sticks.fill(glm::ivec3(0));
		_buttons.fill(false);
		_num = Gamepad::MaxAmount;
	}



	void GamepadImpl::setAxis(GamepadAxis axis, int value) noexcept
	{
		switch (axis)
		{
			case GamepadAxis::LeftX:
				_sticks[to_underlying(GamepadStick::Left)].x = value;
				break;
			case GamepadAxis::LeftY:
				_sticks[to_underlying(GamepadStick::Left)].y = value;
				break;
			case GamepadAxis::LeftZ:
				_sticks[to_underlying(GamepadStick::Left)].z = value;
				break;
			case GamepadAxis::RightX:
				_sticks[to_underlying(GamepadStick::Right)].x = value;
				break;
			case GamepadAxis::RightY:
				_sticks[to_underlying(GamepadStick::Right)].y = value;
				break;
			case GamepadAxis::RightZ:
				_sticks[to_underlying(GamepadStick::Right)].z = value;
				break;
			default:
				break;
		}
	}

	void GamepadImpl::setButton(GamepadButton button, bool down) noexcept
	{
		_buttons[to_underlying(button)] = down;
	}

	const glm::ivec3& GamepadImpl::getStick(GamepadStick stick) const noexcept
	{
		return _sticks[to_underlying(stick)];
	}

	bool GamepadImpl::getButton(GamepadButton button) const noexcept
	{
		return _buttons[to_underlying(button)];
	}

	const GamepadButtons& GamepadImpl::getButtons() const noexcept
	{
		return _buttons;
	}

	const GamepadSticks& GamepadImpl::getSticks() const noexcept
	{
		return _sticks;
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

	const glm::ivec3& Gamepad::getStick(GamepadStick stick) const noexcept
	{
		return _impl->getStick(stick);
	}

	bool Gamepad::getButton(GamepadButton button) const noexcept
	{
		return _impl->getButton(button);
	}

	const GamepadSticks& Gamepad::getSticks() const noexcept
	{
		return _impl->getSticks();
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

	std::optional<KeyboardKey> KeyboardBindingKey::readKey(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);
		static const std::string keyPrefix = "key";
		std::string keyStr;
		if (lowerName.starts_with(keyPrefix))
		{
			keyStr = lowerName.substr(keyPrefix.size());
		}
		for (auto i = 0; i < BX_COUNTOF(s_keyboardKeyNames); i++)
		{
			auto keyName = strToLower(s_keyboardKeyNames[i]);
			if (keyName == lowerName || keyName == keyStr)
			{
				return (KeyboardKey)i;
			}
		}
		return std::nullopt;
	}

	static const std::unordered_map<std::string, uint8_t> s_keyboardModifierNames =
	{
		{ "leftalt", to_underlying(KeyboardModifier::LeftAlt) },
		{ "rightalt", to_underlying(KeyboardModifier::RightAlt) },
		{ "leftctrl", to_underlying(KeyboardModifier::LeftCtrl) },
		{ "rightctrl", to_underlying(KeyboardModifier::RightCtrl) },
		{ "leftshift", to_underlying(KeyboardModifier::LeftShift) },
		{ "rightshift", to_underlying(KeyboardModifier::RightShift) },
		{ "leftmeta", to_underlying(KeyboardModifier::LeftMeta) },
		{ "rightmeta", to_underlying(KeyboardModifier::RightMeta) },
	};

	uint8_t KeyboardBindingKey::readModifiers(std::string_view name) noexcept
	{
		static const char sep = '+';
		auto lowerName = strToLower(name);
		uint8_t modifiers = 0;
		size_t pos = 0;
		while (pos < std::string::npos)
		{
			auto newPos = lowerName.find(sep, pos);
			auto modStr = lowerName.substr(pos, newPos - pos);

			auto itr = s_keyboardModifierNames.find(modStr);
			if (itr != s_keyboardModifierNames.end())
			{
				modifiers |= itr->second;
			}
			pos = newPos;
		}
		return modifiers;
	}

	std::optional<KeyboardBindingKey> KeyboardBindingKey::read(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);

		static const std::string prefix = "keyboard";
		if (lowerName.starts_with(prefix))
		{
			lowerName = lowerName.substr(prefix.size());
		}

		static const char sep = '+';
		auto pos = lowerName.find(sep);
		auto key = readKey(lowerName.substr(0, pos));
		if (!key)
		{
			return std::nullopt;
		}
		
		uint8_t modifiers = 0;
		if (pos != std::string::npos)
		{
			modifiers = readModifiers(lowerName.substr(pos));
		}
		
		return KeyboardBindingKey{ key.value(), modifiers};
	}

	size_t MouseBindingKey::hash() const noexcept
	{
		return to_underlying(button);
	}

	std::optional<MouseButton> MouseBindingKey::readButton(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);
		for (auto i = 0; i < BX_COUNTOF(s_mouseButtonNames); i++)
		{
			if (strToLower(s_mouseButtonNames[i]) == lowerName)
			{
				return (MouseButton)i;
			}
		}
		return std::nullopt;
	}

	std::optional<MouseBindingKey> MouseBindingKey::read(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);
		static const std::string prefix = "mouse";
		if (!lowerName.starts_with(prefix))
		{
			return std::nullopt;
		}

		auto button = readButton(lowerName.substr(prefix.size()));
		if (button)
		{
			return MouseBindingKey{ button.value() };
		}
		return std::nullopt;
	}

	size_t GamepadBindingKey::hash() const noexcept
	{
		return ((size_t)to_underlying(button)) << gamepad;
	}

	std::optional<GamepadButton> GamepadBindingKey::readButton(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);
		for (auto i = 0; i < BX_COUNTOF(s_mouseButtonNames); i++)
		{
			if (strToLower(s_gamepadButtonNames[i]) == lowerName)
			{
				return (GamepadButton)i;
			}
		}
		return std::nullopt;
	}

	std::optional<GamepadBindingKey> GamepadBindingKey::read(std::string_view name) noexcept
	{
		auto lowerName = strToLower(name);
		static const std::string prefix = "gamepad";
		if (!lowerName.starts_with(prefix))
		{
			return std::nullopt;
		}
		lowerName = lowerName.substr(prefix.size());
		uint8_t gamepad = 0;
		static const char sep = ':';
		auto pos = lowerName.find(sep);
		if (pos != std::string::npos)
		{
			gamepad = std::stoi(lowerName.substr(0, pos));
			lowerName = lowerName.substr(pos);
		}
		auto button = readButton(lowerName);
		if (button)
		{
			return GamepadBindingKey{ gamepad, button.value() };
		}
		return std::nullopt;
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

	std::optional<InputBindingKey> InputBinding::readKey(std::string_view name) noexcept
	{
		auto kb = KeyboardBindingKey::read(name);
		if (kb)
		{
			return kb;
		}
		auto mouse = MouseBindingKey::read(name);
		if (mouse)
		{
			return mouse;
		}
		auto gamepad = GamepadBindingKey::read(name);
		if (gamepad)
		{
			return gamepad;
		}
		return std::nullopt;
	}

	std::optional<InputBinding> InputBinding::read(std::string_view name, std::function<void()>&& fn) noexcept
	{
		static const std::string onceSuffix = "once";
		auto lowerName = strToLower(name);
		bool once = false;
		if (lowerName.ends_with(onceSuffix))
		{
			once = true;
			lowerName = lowerName.substr(0, lowerName.size() - onceSuffix.size());
		}
		auto key = readKey(lowerName);
		if (!key)
		{
			return std::nullopt;
		}
		return InputBinding{ key.value(), once, std::move(fn)};
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

	void InputImpl::processBindings() noexcept
	{
		for (auto& elm : _bindings)
		{
			for (auto& binding : elm.second)
			{
				processBinding(binding);
			}
		}
	}

	void InputImpl::resetOnceBindings() noexcept
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

	void InputImpl::clearBindings() noexcept
	{
		_bindings.clear();
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

	void InputImpl::update() noexcept
	{
		_keyboard.getImpl().update();
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

	void Input::processBindings() noexcept
	{
		_impl->processBindings();
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