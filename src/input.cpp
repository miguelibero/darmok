
#include "input.hpp"
#include <bx/bx.h>
#include <darmok/window.hpp>

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

	bool KeyboardImpl::setKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept
	{
		auto k = to_underlying(key);
		if (k > _keys.size())
		{
			return false;
		}
		auto& oldValue = _keys[k];
		auto value = encodeKey(down, modifiers);
		if (oldValue == value)
		{
			return false;
		}
		_keys[k] = value;
		for (auto& listener : _listeners)
		{
			listener->onKeyboardKey(key, modifiers, down);
		}
		return true;
	}

	void KeyboardImpl::pushChar(const Utf8Char& data) noexcept
	{
		_chars[_charsWrite] = data;
		_charsWrite = (_charsWrite + 1) % _chars.size();
		for (auto& listener : _listeners)
		{
			listener->onKeyboardChar(data);
		}
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

	void KeyboardImpl::addListener(IKeyboardListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool KeyboardImpl::removeListener(IKeyboardListener& listener) noexcept
	{
		return _listeners.erase(listener) > 0;
	}

	static const std::array<std::string, to_underlying(KeyboardKey::Count)> s_keyboardKeyNames =
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
		"GraveAccent",
		"CapsLock",
		"NumLock",
		"ScrollLock",
		"Pause",
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

	const std::string& Keyboard::getKeyName(KeyboardKey key) noexcept
	{
		auto idx = to_underlying(key);
		if (idx >= s_keyboardKeyNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return s_keyboardKeyNames[idx];
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
			const bool shift = !!( modifiers & to_underlying(KeyboardModifierGroup::Shift) );
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

	void Keyboard::addListener(IKeyboardListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Keyboard::removeListener(IKeyboardListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

#pragma endregion Keyboard

#pragma region Mouse

	MouseImpl::MouseImpl() noexcept
		: _buttons{}
		, _position(0)
		, _lastPosition(0)
		, _scroll(0)
		, _lastScroll(0)
		, _active(false)
		, _hasBeenInactive(true)
	{
	}

	bool MouseImpl::setActive(bool active) noexcept
	{
		if (_active == active)
		{
			return false;
		}
		_active = active;
		if (!active)
		{
			_hasBeenInactive = true;
		}
		for (auto& listener : _listeners)
		{
			listener->onMouseActive(active);
		}
		return true;
	}

	bool MouseImpl::setPosition(const glm::vec2& pos) noexcept
	{
		if (_position == pos)
		{
			return false;
		}
		if (_hasBeenInactive)
		{
			_lastPosition = pos;
		}
		auto delta = pos - _position;
		_position = pos;
		if (!_hasBeenInactive)
		{
			for (auto& listener : _listeners)
			{
				listener->onMousePositionChange(delta, _position);
			}
		}
		return true;
	}

	bool MouseImpl::setScroll(const glm::vec2& scroll) noexcept
	{
		if (_scroll == scroll)
		{
			return false;
		}
		auto delta = scroll - _scroll;
		_scroll = scroll;
		for (auto& listener : _listeners)
		{
			listener->onMouseScrollChange(delta, _scroll);
		}
		return true;
	}

	bool MouseImpl::setButton(MouseButton button, bool down) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttons.size() || _buttons[idx] == down)
		{
			return false;
		}
		_buttons[idx] = down;
		for (auto& listener : _listeners)
		{
			listener->onMouseButton(button, down);
		}
		return true;
	}

	void MouseImpl::update() noexcept
	{
		_lastPosition = _position;
		_lastScroll = _scroll;
		if (_active)
		{
			_hasBeenInactive = false;
		}
	}

	bool MouseImpl::getButton(MouseButton button) const noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttons.size())
		{
			return false;
		}
		return _buttons[idx];
	}

	bool MouseImpl::getActive() const noexcept
	{
		return _active;
	}

	const glm::vec2& MouseImpl::getPosition() const noexcept
	{
		return _position;
	}

	glm::vec2 MouseImpl::getPositionDelta() const noexcept
	{
		if (_hasBeenInactive)
		{
			return glm::vec2(0);
		}
		return _position - _lastPosition;
	}

	const glm::vec2& MouseImpl::getScroll() const noexcept
	{
		return _scroll;
	}

	glm::vec2 MouseImpl::getScrollDelta() const noexcept
	{
		return _scroll - _lastScroll;
	}

	const MouseButtons& MouseImpl::getButtons() const noexcept
	{
		return _buttons;
	}

	void MouseImpl::addListener(IMouseListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool MouseImpl::removeListener(IMouseListener& listener) noexcept
	{
		return _listeners.erase(listener) > 0;
	}

	static const std::array<std::string, to_underlying(MouseButton::Count)> s_mouseButtonNames =
	{
		"Left",
		"Middle",
		"Right",
	};

	const std::string& Mouse::getButtonName(MouseButton button) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= s_mouseButtonNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return s_mouseButtonNames[idx];
	}

	Mouse::Mouse() noexcept
		: _impl(std::make_unique<MouseImpl>())
	{
	}

	bool Mouse::getButton(MouseButton button) const noexcept
	{
		return _impl->getButton(button);
	}

	const glm::vec2& Mouse::getPosition() const noexcept
	{
		return _impl->getPosition();
	}

	glm::vec2 Mouse::getPositionDelta() const noexcept
	{
		return _impl->getPositionDelta();
	}

	const glm::vec2& Mouse::getScroll() const noexcept
	{
		return _impl->getScroll();
	}

	glm::vec2 Mouse::getScrollDelta() const noexcept
	{
		return _impl->getScrollDelta();
	}

	bool Mouse::getActive() const noexcept
	{
		return _impl->getActive();
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

	void Mouse::addListener(IMouseListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Mouse::removeListener(IMouseListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

#pragma endregion Mouse

#pragma region Gamepad

	GamepadImpl::GamepadImpl() noexcept
		: _num(Gamepad::MaxAmount)
		, _connected(false)
		, _sticks{}
		, _buttons{}
	{
	}

	bool GamepadImpl::setNumber(uint8_t num) noexcept
	{
		if (_num == num)
		{
			return false;
		}
		_num = num;
		clear();
		return true;
	}
	bool GamepadImpl::setConnected(bool value) noexcept
	{
		if (_connected == value)
		{
			return false;
		}
		clear();
		for (auto& listener : _listeners)
		{
			listener->onGamepadConnect(_num, value);
		}
		return true;
	}

	void GamepadImpl::clear() noexcept
	{
		_sticks.fill(glm::ivec3(0));
		_buttons.fill(false);
	}

	bool GamepadImpl::setStick(GamepadStick stick, const glm::ivec3& value) noexcept
	{
		auto idx = to_underlying(stick);
		if (idx >= _sticks.size())
		{
			return false;
		}
		auto& oldValue = _sticks[idx];
		if (oldValue == value)
		{
			return false;
		}
		_sticks[idx] = value;
		auto delta = value - oldValue;
		for (auto& listener : _listeners)
		{
			listener->onGamepadStickChange(_num, stick, delta, value);
		}
		return true;
	}

	bool GamepadImpl::setButton(GamepadButton button, bool down) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttons.size() || _buttons[idx] == down)
		{
			return false;
		}
		_buttons[idx] = down;
		for (auto& listener : _listeners)
		{
			listener->onGamepadButton(_num, button, down);
		}
		return true;
	}

	const glm::ivec3& GamepadImpl::getStick(GamepadStick stick) const noexcept
	{
		auto idx = to_underlying(stick);
		if (idx >= _sticks.size())
		{
			const static glm::ivec3 zero(0);
			return zero;
		}
		return _sticks[idx];
	}

	bool GamepadImpl::getButton(GamepadButton button) const noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttons.size())
		{
			return false;
		}
		return _buttons[idx];
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

	void GamepadImpl::addListener(IGamepadListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool GamepadImpl::removeListener(IGamepadListener& listener) noexcept
	{
		return _listeners.erase(listener) > 0;
	}

	static const std::array<std::string, to_underlying(GamepadButton::Count)> s_gamepadButtonNames =
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
		"Guide"
	};

	Gamepad::Gamepad() noexcept
		: _impl(std::make_unique<GamepadImpl>())
	{
	}

	const std::string& Gamepad::getButtonName(GamepadButton button) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= s_gamepadButtonNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return s_gamepadButtonNames[idx];
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

	void Gamepad::addListener(IGamepadListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Gamepad::removeListener(IGamepadListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

#pragma endregion Gamepad

#pragma region Input

	size_t KeyboardBindingKey::hash() const noexcept
	{
		return to_underlying(key) | modifiers << 16;
	}

	std::optional<KeyboardKey> KeyboardBindingKey::readKey(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		static const std::string keyPrefix = "key";
		std::string keyStr;
		if (StringUtils::startsWith(lowerName, keyPrefix))
		{
			keyStr = lowerName.substr(keyPrefix.size());
		}
		for (auto i = 0; i < s_keyboardKeyNames.size(); i++)
		{
			auto keyName = StringUtils::toLower(s_keyboardKeyNames[i]);
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
		auto lowerName = StringUtils::toLower(name);
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
		auto lowerName = StringUtils::toLower(name);

		static const std::string prefix = "keyboard";
		if (StringUtils::startsWith(lowerName, prefix))
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
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < s_mouseButtonNames.size(); i++)
		{
			if (StringUtils::toLower(s_mouseButtonNames[i]) == lowerName)
			{
				return (MouseButton)i;
			}
		}
		return std::nullopt;
	}

	std::optional<MouseBindingKey> MouseBindingKey::read(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		static const std::string prefix = "mouse";
		if (!StringUtils::startsWith(lowerName, prefix))
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
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < s_mouseButtonNames.size(); i++)
		{
			if (StringUtils::toLower(s_gamepadButtonNames[i]) == lowerName)
			{
				return (GamepadButton)i;
			}
		}
		return std::nullopt;
	}

	std::optional<GamepadBindingKey> GamepadBindingKey::read(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		static const std::string prefix = "gamepad";
		if (!StringUtils::startsWith(lowerName, prefix))
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

		constexpr size_t maxKey = to_underlying(KeyboardKey::Count) + UINT8_MAX;

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
		auto lowerName = StringUtils::toLower(name);
		bool once = false;
		if (StringUtils::endsWith(lowerName, onceSuffix))
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
		uint8_t num = 0;
		for (auto& gamepad : _gamepads)
		{
			gamepad.getImpl().setNumber(num++);
		}
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
			if (gamepad)
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
		_mouse.getImpl().update();
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
		return _impl->getGamepad(num);
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