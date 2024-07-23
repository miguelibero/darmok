
#include "input.hpp"
#include <bx/bx.h>
#include <darmok/window.hpp>
#include <darmok/string.hpp>

namespace darmok
{
	template<typename T, size_t S>
	static const std::string& getInputEnumName(T v, const std::array<std::string, S>& names) noexcept
	{
		auto idx = to_underlying(v);
		if (idx >= names.size())
		{
			static const std::string empty;
			return empty;
		}
		return names[idx];
	}

	template<typename T, size_t S>
	static std::optional<T> readInputEnum(std::string_view name, const std::array<std::string, S>& names) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < names.size(); i++)
		{
			auto keyName = StringUtils::toLower(names[i]);
			if (keyName == lowerName)
			{
				return (T)i;
			}
		}
		return std::nullopt;
	}

#pragma region Keyboard

	KeyboardImpl::KeyboardImpl() noexcept
		: _keys{}
		, _charsRead(0)
		, _charsWrite(0)
	{
	}

	void KeyboardImpl::reset() noexcept
	{
		flush();
	}

	void KeyboardImpl::setKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept
	{
		bool changed = false;
		if (down)
		{
			if (_keys.insert(key).second)
			{
				changed = true;
			}
		}
		else
		{
			if (_keys.erase(key))
			{
				changed = true;
			}
		}
		if (_modifiers != modifiers)
		{
			_modifiers = modifiers;
			changed = true;
		}
		if (changed)
		{
			for (auto& listener : _listeners)
			{
				listener->onKeyboardKey(key, modifiers, down);
			}
		}
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
		return _keys.contains(key);
	}

	const KeyboardKeys& KeyboardImpl::getKeys() const noexcept
	{
		return _keys;
	}

	Utf8Char KeyboardImpl::popChar() noexcept
	{
		if (_charsRead == _charsWrite)
		{
			return {};
		}
		auto& v = _chars[_charsRead];
		_charsRead = (_charsRead + 1) % _chars.size();
		return v;
	}

	const KeyboardModifiers& KeyboardImpl::getModifiers() const noexcept
	{
		return _modifiers;
	}

	bool KeyboardImpl::hasModifier(KeyboardModifier mod) const noexcept
	{
		return _modifiers.contains(mod);
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
			if (!c)
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

	char KeyboardImpl::keyToAscii(KeyboardKey key, bool upper) noexcept
	{
		const bool isAscii = (KeyboardKey::Key0 <= key && key <= KeyboardKey::KeyZ)
			|| (KeyboardKey::Esc <= key && key <= KeyboardKey::Minus);
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
			return (upper ? 'A' : 'a') + char(to_underlying(key) - to_underlying(KeyboardKey::KeyA));
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

	const std::array<std::string, to_underlying(KeyboardKey::Count)> KeyboardImpl::_keyNames =
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

	const std::string& KeyboardImpl::getKeyName(KeyboardKey key) noexcept
	{
		return getInputEnumName(key, _keyNames);
	}

	std::optional<KeyboardKey> KeyboardImpl::readKey(std::string_view name) noexcept
	{
		return readInputEnum<KeyboardKey>(name, _keyNames);
	}

	const std::array<std::string, to_underlying(KeyboardModifier::Count)> KeyboardImpl::_modNames
	{
		"Alt",
		"Ctrl",
		"Shift",
		"Meta"
	};

	const std::string& KeyboardImpl::getModifierName(KeyboardModifier mod) noexcept
	{
		return getInputEnumName(mod, _modNames);
	}

	std::optional<KeyboardModifier> KeyboardImpl::readModifier(std::string_view name) noexcept
	{
		return readInputEnum<KeyboardModifier>(name, _modNames);
	}

	std::optional<KeyboardInputEvent> KeyboardImpl::readEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split("+", name);
		auto key = Keyboard::readKey(parts[0]);
		if (!key)
		{
			return std::nullopt;
		}
		KeyboardModifiers mods;
		for (size_t i = 1; i < parts.size(); i++)
		{
			auto mod = Keyboard::readModifier(parts[i]);
			if (mod)
			{
				mods.insert(mod.value());
			}
		}
		return KeyboardInputEvent{ key.value(), mods };
	}

	Keyboard::Keyboard() noexcept
		: _impl(std::make_unique<KeyboardImpl>())
	{
	}

	Keyboard::~Keyboard() noexcept
	{
		// left empty to get the forward declaration of the impl working
	}	

	bool Keyboard::getKey(KeyboardKey key) const noexcept
	{
		return _impl->getKey(key);
	}

	const KeyboardKeys& Keyboard::getKeys() const noexcept
	{
		return _impl->getKeys();
	}

	const KeyboardChars& Keyboard::getUpdateChars() const noexcept
	{
		return _impl->getUpdateChars();
	}

	const KeyboardModifiers& Keyboard::getModifiers() const noexcept
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

	char Keyboard::keyToAscii(KeyboardKey key, bool upper) noexcept
	{
		return KeyboardImpl::keyToAscii(key, upper);
	}

	const std::string& Keyboard::getKeyName(KeyboardKey key) noexcept
	{
		return KeyboardImpl::getKeyName(key);
	}

	std::optional<KeyboardKey> Keyboard::readKey(std::string_view name) noexcept
	{
		return KeyboardImpl::readKey(name);
	}

	std::optional<KeyboardModifier> Keyboard::readModifier(std::string_view name) noexcept
	{
		return KeyboardImpl::readModifier(name);
	}

	const std::string& Keyboard::getModifierName(KeyboardModifier mod) noexcept
	{
		return KeyboardImpl::getModifierName(mod);
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

	glm::vec2 MouseImpl::getDelta(MouseAnalog analog) const noexcept
	{
		switch (analog)
		{
		case MouseAnalog::Scroll:
			return getScrollDelta();
		case MouseAnalog::Position:
			return getPositionDelta();
		}
		return glm::vec2(0);
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

	const std::array<std::string, to_underlying(MouseButton::Count)> MouseImpl::_buttonNames =
	{
		"Left",
		"Middle",
		"Right",
	};

	std::optional<MouseButton> MouseImpl::readButton(std::string_view name) noexcept
	{
		return readInputEnum<MouseButton>(name, _buttonNames);
	}

	const std::string& MouseImpl::getButtonName(MouseButton button) noexcept
	{
		return getInputEnumName(button, _buttonNames);
	}

	const std::array<std::string, to_underlying(MouseAnalog::Count)> MouseImpl::_analogNames =
	{
		"Position",
		"Scroll"
	};

	std::optional<MouseAnalog> MouseImpl::readAnalog(std::string_view name) noexcept
	{
		return readInputEnum<MouseAnalog>(name, _analogNames);
	}

	const std::string& MouseImpl::getAnalogName(MouseAnalog analog) noexcept
	{
		return getInputEnumName(analog, _analogNames);
	}

	std::optional<MouseInputEvent> MouseImpl::readEvent(std::string_view name) noexcept
	{
		if (auto button = readButton(name))
		{
			return MouseInputEvent{ button.value() };
		}
		return std::nullopt;
	}

	std::optional<MouseInputDir> MouseImpl::readDir(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(":", name);
		auto size = parts.size();
		if (size < 2)
		{
			return std::nullopt;
		}
		auto analog = readAnalog(parts[0]);
		if (!analog)
		{
			return std::nullopt;
		}
		auto type = InputImpl::readDirType(parts[1]);
		if (!type)
		{
			return std::nullopt;
		}
		return MouseInputDir{ analog.value(), type.value() };
	}

	std::optional<MouseButton> Mouse::readButton(std::string_view name) noexcept
	{
		return MouseImpl::readButton(name);
	}

	const std::string& Mouse::getButtonName(MouseButton button) noexcept
	{
		return MouseImpl::getButtonName(button);
	}

	std::optional<MouseAnalog> Mouse::readAnalog(std::string_view name) noexcept
	{
		return MouseImpl::readAnalog(name);
	}

	const std::string& Mouse::getAnalogName(MouseAnalog analog) noexcept
	{
		return MouseImpl::getAnalogName(analog);
	}

	Mouse::Mouse() noexcept
		: _impl(std::make_unique<MouseImpl>())
	{
	}

	Mouse::~Mouse() noexcept
	{
		// left empty to get the forward declaration of the impl working
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

	bool GamepadImpl::setStick(GamepadStick stick, const glm::vec3& value) noexcept
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

	const glm::vec3& GamepadImpl::getStick(GamepadStick stick) const noexcept
	{
		auto idx = to_underlying(stick);
		if (idx >= _sticks.size())
		{
			const static glm::vec3 zero(0);
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

	const std::array<std::string, to_underlying(GamepadButton::Count)> GamepadImpl::_buttonNames =
	{
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

	std::optional<GamepadButton> GamepadImpl::readButton(std::string_view name) noexcept
	{
		return readInputEnum<GamepadButton>(name, _buttonNames);
	}

	const std::string& GamepadImpl::getButtonName(GamepadButton button) noexcept
	{
		return getInputEnumName(button, _buttonNames);
	}

	const std::array<std::string, to_underlying(GamepadStick::Count)> GamepadImpl::_stickNames =
	{
		"Left",
		"Right"
	};

	std::optional<GamepadStick> GamepadImpl::readStick(std::string_view name) noexcept
	{
		return readInputEnum<GamepadStick>(name, _stickNames);
	}

	const std::string& GamepadImpl::getStickName(GamepadStick stick) noexcept
	{
		return getInputEnumName(stick, _stickNames);
	}

	std::optional<uint8_t> GamepadImpl::readNum(std::string_view name) noexcept
	{
		auto ul = std::stoul(std::string(name));
		if (ul > std::numeric_limits<uint8_t>::max())
		{
			return Gamepad::Any;
		}
		return static_cast<uint8_t>(ul);
	}

	std::optional<GamepadInputEvent> GamepadImpl::readEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(":", name);
		auto size = parts.size();
		if (size == 0)
		{
			return std::nullopt;
		}
		auto button = readButton(parts[0]);
		if (!button)
		{
			return std::nullopt;
		}
		auto gamepad = readNum(parts[1]).value_or(Gamepad::Any);
		return GamepadInputEvent{ button.value(), gamepad };
	}

	std::optional<GamepadInputDir> GamepadImpl::readDir(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(":", name);
		auto size = parts.size();
		if (size < 2)
		{
			return std::nullopt;
		}
		auto stick = readStick(parts[0]);
		if (!stick)
		{
			return std::nullopt;
		}
		auto type = InputImpl::readDirType(parts[1]);
		if (!type)
		{
			return std::nullopt;
		}
		auto gamepad = readNum(parts[1]).value_or(Gamepad::Any);
		return GamepadInputDir{ stick.value(), type.value(), gamepad };
	}

	Gamepad::Gamepad() noexcept
		: _impl(std::make_unique<GamepadImpl>())
	{
	}

	Gamepad::~Gamepad() noexcept
	{
		// left empty to get the forward declaration of the impl working
	}

	std::optional<GamepadButton> Gamepad::readButton(std::string_view name) noexcept
	{
		return GamepadImpl::readButton(name);
	}

	std::optional<GamepadStick> Gamepad::readStick(std::string_view name) noexcept
	{
		return GamepadImpl::readStick(name);
	}

	const std::string& Gamepad::getButtonName(GamepadButton button) noexcept
	{
		return GamepadImpl::getButtonName(button);
	}

	const std::string& Gamepad::getStickName(GamepadStick stick) noexcept
	{
		return GamepadImpl::getStickName(stick);
	}

	const glm::vec3& Gamepad::getStick(GamepadStick stick) const noexcept
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

#pragma region Events

	bool KeyboardInputEvent::operator==(const KeyboardInputEvent& other) const noexcept
	{
		return key == other.key && modifiers == other.modifiers;
	}

	bool MouseInputEvent::operator==(const MouseInputEvent& other) const noexcept
	{
		return button == other.button;
	}

	bool GamepadInputEvent::operator==(const GamepadInputEvent& other) const noexcept
	{
		return button == other.button && gamepad == other.gamepad;
	}

	bool MouseInputDir::operator==(const MouseInputDir& other) const noexcept
	{
		return analog == other.analog && type == other.type;
	}

	bool GamepadInputDir::operator==(const GamepadInputDir& other) const noexcept
	{
		return stick == other.stick && type == other.type && gamepad == other.gamepad;
	}

	bool operator==(const InputEvent& a, const InputEvent& b) noexcept
	{
		if (auto v1 = std::get_if<KeyboardInputEvent>(&a))
		{
			if (auto v2 = std::get_if<KeyboardInputEvent>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		if (auto v1 = std::get_if<MouseInputEvent>(&a))
		{
			if (auto v2 = std::get_if<MouseInputEvent>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		if (auto v1 = std::get_if<GamepadInputEvent>(&a))
		{
			if (auto v2 = std::get_if<GamepadInputEvent>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		return false;
	}

	bool operator==(const InputDir& a, const InputDir& b) noexcept
	{
		if (auto v1 = std::get_if<MouseInputDir>(&a))
		{
			if (auto v2 = std::get_if<MouseInputDir>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		if (auto v1 = std::get_if<GamepadInputDir>(&a))
		{
			if (auto v2 = std::get_if<GamepadInputDir>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		if (auto v1 = std::get_if<InputEvent>(&a))
		{
			if (auto v2 = std::get_if<InputEvent>(&b))
			{
				return *v1 == *v2;
			}
			return false;
		}
		return false;
	}

#pragma endregion Events

#pragma region Input

	InputImpl::InputImpl(Input& input) noexcept
		: _input(input)
	{
		uint8_t num = 0;
		for (auto& gamepad : _gamepads)
		{
			gamepad.getImpl().setNumber(num++);
			gamepad.addListener(*this);
		}
		_keyboard.addListener(*this);
		_mouse.addListener(*this);
	}

	void InputImpl::addListener(const std::string& tag, const InputEvent& ev, IInputEventListener& listener) noexcept
	{
		_listeners.push_back(ListenerData{ tag, ev, listener });
	}

	bool InputImpl::removeListener(const std::string& tag, IInputEventListener& listener) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&tag, &listener](auto& data)
		{
			return data.tag == tag && data.listener == listener;
		});

		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool InputImpl::removeListener(IInputEventListener& listener) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&listener](auto& data)
		{
			return data.listener == listener;
		});
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool InputImpl::checkEvent(const InputEvent& ev) const noexcept
	{
		if (auto v = std::get_if<KeyboardInputEvent>(&ev))
		{
			if (!_keyboard.getKey(v->key))
			{
				return false;
			}
			return _keyboard.getModifiers() == v->modifiers;
		}
		if (auto v = std::get_if<MouseInputEvent>(&ev))
		{
			return _mouse.getButton(v->button);
		}
		if (auto v = std::get_if<GamepadInputEvent>(&ev))
		{
			auto gamepad = getGamepad(v->gamepad);
			if(!gamepad)
			{
				return false;
			}
			return gamepad->getButton(v->button);
		}
		return false;
	}

	float InputImpl::getDir(const glm::vec2& vec, InputDirType dir) noexcept
	{
		float v = 0.F;
		switch (dir)
		{
			case InputDirType::Up:
				v = vec.y;
				break;
			case InputDirType::Down:
				v = -vec.y;
				break;
			case InputDirType::Left:
				v = -vec.x;
				break;
			case InputDirType::Right:
				v = vec.x;
				break;
		}
		return v;
	}

	float InputImpl::getDir(const InputDir& dir) const noexcept
	{
		if (auto v = std::get_if<MouseInputDir>(&dir))
		{
			glm::vec2 vec = _mouse.getImpl().getDelta(v->analog);
			return getDir(vec, v->type);
		}
		if (auto v = std::get_if<GamepadInputDir>(&dir))
		{
			auto gamepad = getGamepad(v->gamepad);
			if (!gamepad)
			{
				return 0.F;
			}
			return getDir(gamepad->getStick(v->stick), v->type);
		}
		if (auto v = std::get_if<InputEvent>(&dir))
		{
			return checkEvent(*v) ? 1.F : 0.F;
		}
		return 0.F;
	}

	float InputImpl::getAxis(const std::vector<InputDir>& positive, const std::vector<InputDir>& negative) const noexcept
	{
		float v = 0;
		for (auto& dir : positive)
		{
			v += getDir(dir);
		}
		for (auto& dir : negative)
		{
			v -= getDir(dir);
		}
		return v;
	}

	void InputImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down)
	{
		if (!down)
		{
			return;
		}
		for (auto& data : _listeners)
		{
			auto ev = std::get_if<KeyboardInputEvent>(&data.event);
			if (!ev)
			{
				continue;
			}
			if (ev->key == key && ev->modifiers == modifiers)
			{
				data.listener->onInputEvent(data.tag);
			}
		}
	}

	void InputImpl::onMouseButton(MouseButton button, bool down)
	{
		if (!down)
		{
			return;
		}
		for (auto& data : _listeners)
		{
			auto ev = std::get_if<MouseInputEvent>(&data.event);
			if (!ev)
			{
				continue;
			}
			if (ev->button == button)
			{
				data.listener->onInputEvent(data.tag);
			}
		}
	}

	void InputImpl::onGamepadButton(uint8_t num, GamepadButton button, bool down)
	{
		if (!down)
		{
			return;
		}
		for (auto& data : _listeners)
		{
			auto ev = std::get_if<GamepadInputEvent>(&data.event);
			if (!ev)
			{
				continue;
			}
			if ((ev->gamepad == num || ev->gamepad == Gamepad::Any) && ev->button == button)
			{
				data.listener->onInputEvent(data.tag);
			}
		}
	}

	const std::array<std::string, to_underlying(InputDirType::Count)> InputImpl::_dirTypeNames =
	{
		"Up",
		"Down",
		"Left",
		"Right"
	};

	std::optional<InputDirType> InputImpl::readDirType(std::string_view name) noexcept
	{
		return readInputEnum<InputDirType>(name, _dirTypeNames);
	}

	const std::string& InputImpl::getDirTypeName(InputDirType type) noexcept
	{
		return getInputEnumName(type, _dirTypeNames);
	}

	const std::string InputImpl::_keyboardPrefix = "keyboard:";
	const std::string InputImpl::_mousePrefix = "mouse:";
	const std::string InputImpl::_gamepadPrefix = "gamepad:";

	std::optional<InputEvent> InputImpl::readEvent(std::string_view name) noexcept
	{
		if (name.starts_with(_keyboardPrefix))
		{
			return KeyboardImpl::readEvent(name.substr(_keyboardPrefix.size()));
		}
		if (name.starts_with(_mousePrefix))
		{
			return MouseImpl::readEvent(name.substr(_mousePrefix.size()));
		}
		if (name.starts_with(_gamepadPrefix))
		{
			return GamepadImpl::readEvent(name.substr(_gamepadPrefix.size()));
		}
		return std::nullopt;
	}

	std::optional<InputDir> InputImpl::readDir(std::string_view name) noexcept
	{
		if (name.starts_with(_gamepadPrefix))
		{
			if (auto dir = GamepadImpl::readDir(name.substr(_gamepadPrefix.size())))
			{
				return dir;
			}
		}
		if (name.starts_with(_mousePrefix))
		{
			if (auto dir = GamepadImpl::readDir(name.substr(_mousePrefix.size())))
			{
				return dir;
			}
		}
		return readEvent(name);
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
		if (num > 0 || num < Gamepad::MaxAmount)
		{
			return _gamepads[num];
		}
		return nullptr;
	}

	Gamepads& InputImpl::getGamepads() noexcept
	{
		return _gamepads;
	}

	const Keyboard& InputImpl::getKeyboard() const noexcept
	{
		return _keyboard;
	}

	const Mouse& InputImpl::getMouse() const noexcept
	{
		return _mouse;
	}

	OptionalRef<const Gamepad> InputImpl::getGamepad(uint8_t num) const noexcept
	{
		if (num == Gamepad::Any)
		{
			for (auto& gamepad : _gamepads)
			{
				if (gamepad.isConnected())
				{
					return gamepad;
				}
			}
			return nullptr;
		}
		if (num > 0 || num < Gamepad::MaxAmount)
		{
			return _gamepads[num];
		}
		return nullptr;
	}

	const Gamepads& InputImpl::getGamepads() const noexcept
	{
		return _gamepads;
	}

	void InputImpl::update() noexcept
	{
		_keyboard.getImpl().update();
		_mouse.getImpl().update();
	}

	Input::Input() noexcept
		: _impl(std::make_unique<InputImpl>(*this))
	{
	}

	Input::~Input() noexcept
	{
		// left empty to get the forward declaration of the impl working
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

	void Input::addListener(const std::string& tag, const InputEvent& ev, IInputEventListener& listener) noexcept
	{
		_impl->addListener(tag, ev, listener);
	}

	bool Input::removeListener(IInputEventListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	bool Input::removeListener(const std::string& tag, IInputEventListener& listener) noexcept
	{
		return _impl->removeListener(tag, listener);
	}

	std::optional<InputEvent> Input::readEvent(std::string_view name) noexcept
	{
		return InputImpl::readEvent(name);
	}

	std::optional<InputDir> Input::readDir(std::string_view name) noexcept
	{
		return InputImpl::readDir(name);
	}

	std::optional<InputDirType> Input::readDirType(std::string_view name) noexcept
	{
		return InputImpl::readDirType(name);
	}

	const std::string& Input::getDirTypeName(InputDirType type) noexcept
	{
		return InputImpl::getDirTypeName(type);
	}

	bool Input::checkEvent(const InputEvent& ev) const noexcept
	{
		return _impl->checkEvent(ev);
	}

	float Input::getAxis(const InputDirs& positive, const InputDirs& negative) const noexcept
	{
		return _impl->getAxis(positive, negative);
	}

#pragma endregion Input

}