
#include "input.hpp"
#include <bx/bx.h>
#include <darmok/window.hpp>
#include <darmok/string.hpp>

namespace darmok
{
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
		if (down)
		{
			_keys.insert(key);
		}
		else
		{
			_keys.erase(key);
		}
		_modifiers = modifiers;

		for (auto& listener : _listeners)
		{
			listener->onKeyboardKey(key, modifiers, down);
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
		auto idx = to_underlying(key);
		if (idx >= _keyNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return _keyNames[idx];
	}

	std::optional<KeyboardKey> KeyboardImpl::readKey(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		static const std::string keyPrefix = "key";
		std::string keyStr;
		if (StringUtils::startsWith(lowerName, keyPrefix))
		{
			keyStr = lowerName.substr(keyPrefix.size());
		}
		for (auto i = 0; i < _keyNames.size(); i++)
		{
			auto keyName = StringUtils::toLower(_keyNames[i]);
			if (keyName == lowerName || keyName == keyStr)
			{
				return (KeyboardKey)i;
			}
		}
		return std::nullopt;
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
		auto idx = to_underlying(mod);
		if (idx >= _modNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return _keyNames[idx];
	}

	std::optional<KeyboardModifier> KeyboardImpl::readModifier(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < _modNames.size(); i++)
		{
			auto keyName = StringUtils::toLower(_modNames[i]);
			if (keyName == lowerName)
			{
				return (KeyboardModifier)i;
			}
		}
		return std::nullopt;
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

	const std::array<std::string, to_underlying(MouseButton::Count)> MouseImpl::_buttonNames =
	{
		"Left",
		"Middle",
		"Right",
	};

	std::optional<MouseButton> MouseImpl::readButton(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < _buttonNames.size(); i++)
		{
			if (StringUtils::toLower(_buttonNames[i]) == lowerName)
			{
				return (MouseButton)i;
			}
		}
		return std::nullopt;
	}

	const std::string& MouseImpl::getButtonName(MouseButton button) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttonNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return _buttonNames[idx];
	}

	std::optional<MouseButton> Mouse::readButton(std::string_view name) noexcept
	{
		return MouseImpl::readButton(name);
	}

	const std::string& Mouse::getButtonName(MouseButton button) noexcept
	{
		return MouseImpl::getButtonName(button);
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

	std::optional<GamepadButton> GamepadImpl::readButton(std::string_view name) noexcept
	{
		auto lowerName = StringUtils::toLower(name);
		for (auto i = 0; i < _buttonNames.size(); i++)
		{
			if (StringUtils::toLower(_buttonNames[i]) == lowerName)
			{
				return (GamepadButton)i;
			}
		}
		return std::nullopt;
	}

	const std::string& GamepadImpl::getButtonName(GamepadButton button) noexcept
	{
		auto idx = to_underlying(button);
		if (idx >= _buttonNames.size())
		{
			static const std::string empty;
			return empty;
		}
		return _buttonNames[idx];
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

	const std::string& Gamepad::getButtonName(GamepadButton button) noexcept
	{
		return GamepadImpl::getButtonName(button);
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

	std::unique_ptr<IInputEvent> IInputEvent::read(const nlohmann::json& json) noexcept
	{
		if (json.is_array())
		{
			return CombinedInputEvent::read(json);
		}
		if (json.is_object())
		{
			auto type = json["type"];
			if (type == "keyboard")
			{
				return KeyboardInputEvent::read(json);
			}
			if (type == "mouse")
			{
				return MouseInputEvent::read(json);
			}
			if (type == "gamepad")
			{
				return GamepadInputEvent::read(json);
			}
			return nullptr;
		}
		auto kb = KeyboardInputEvent::read(json);
		if (kb)
		{
			return kb;
		}
		auto mouse = MouseInputEvent::read(json);
		if (mouse)
		{
			return mouse;
		}
		auto gamepad = GamepadInputEvent::read(json);
		if (gamepad)
		{
			return gamepad;
		}
		return nullptr;
	}

	KeyboardInputEvent::KeyboardInputEvent(KeyboardKey key, const KeyboardModifiers& mods) noexcept
		: key(key), modifiers(mods)
	{
	}

	bool KeyboardInputEvent::check(const Input& input) const noexcept
	{
		auto& kb = input.getKeyboard();
		return kb.getKey(key) && kb.getModifiers() == modifiers;
	}

	std::unique_ptr<IInputEvent> KeyboardInputEvent::copy() const noexcept
	{
		return std::make_unique<KeyboardInputEvent>(key, modifiers);
	}

	std::unique_ptr<KeyboardInputEvent> KeyboardInputEvent::read(const nlohmann::json& json) noexcept
	{
		if (json.is_string())
		{
			auto parts = StringUtils::split("+", json.get<std::string>());
			auto key = Keyboard::readKey(parts[0]);
			if (!key)
			{
				return nullptr;
			}
			KeyboardModifiers mods;
			for (auto i = 0; i < parts.size(); i++)
			{
				auto mod = Keyboard::readModifier(parts[i]);
				if (mod)
				{
					mods.insert(mod.value());
				}
			}
			return std::make_unique<KeyboardInputEvent>(key.value(), mods);
		}

		if (!json.contains("key"))
		{
			return nullptr;
		}
		auto key = Keyboard::readKey(json["key"]);
		if (!key)
		{
			return nullptr;
		}
		KeyboardModifiers mods;
		if(json.contains("modifiers"))
		{
			for (auto& item : json["modifiers"])
			{
				auto mod = Keyboard::readModifier(item);
				if (mod)
				{
					mods.insert(mod.value());
				}
			}
		}
		return std::make_unique<KeyboardInputEvent>(key.value(), mods);
	}

	MouseInputEvent::MouseInputEvent(MouseButton button) noexcept
		: button(button)
	{
	}

	bool MouseInputEvent::check(const Input& input) const noexcept
	{
		return input.getMouse().getButton(button);
	}

	std::unique_ptr<IInputEvent> MouseInputEvent::copy() const noexcept
	{
		return std::make_unique<MouseInputEvent>(button);
	}

	std::unique_ptr<MouseInputEvent> MouseInputEvent::read(const nlohmann::json& json) noexcept
	{
		if (!json.contains("button"))
		{
			return nullptr;
		}
		auto button = Mouse::readButton(json["button"]);
		if (!button)
		{
			return nullptr;
		}
		return std::make_unique<MouseInputEvent>(button.value());
	}

	GamepadInputEvent::GamepadInputEvent(GamepadButton button, uint8_t gamepad) noexcept
		: button(button), gamepad(gamepad)
	{
	}

	bool GamepadInputEvent::check(const Input& input) const noexcept
	{
		auto gp = input.getGamepad(gamepad);
		if (!gp)
		{
			for (auto& elm : input.getGamepads())
			{
				if (elm.isConnected())
				{
					gp = elm;
					break;
				}
			}
		}
		if (!gp)
		{
			return false;
		}
		return gp->getButton(button);
	}

	std::unique_ptr<IInputEvent> GamepadInputEvent::copy() const noexcept
	{
		return std::make_unique<GamepadInputEvent>(button, gamepad);
	}

	std::unique_ptr<GamepadInputEvent> GamepadInputEvent::read(const nlohmann::json& json) noexcept
	{
		if (!json.contains("button"))
		{
			return nullptr;
		}
		auto button = Gamepad::readButton(json["button"]);
		if (!button)
		{
			return nullptr;
		}
		auto gamepad = Gamepad::Any;
		if (json.contains("gamepad"))
		{
			gamepad = json["gamepad"];
		}
		return std::make_unique<GamepadInputEvent>(button.value(), gamepad);
	}

	CombinedInputEvent::CombinedInputEvent(Events&& events) noexcept
		: events(std::move(events))
	{
	}

	bool CombinedInputEvent::check(const Input& input) const noexcept
	{
		for (auto& ev : events)
		{
			if (!ev->check(input))
			{
				return false;
			}
		}
		return true;
	}

	std::unique_ptr<IInputEvent> CombinedInputEvent::copy() const noexcept
	{
		std::vector<std::unique_ptr<IInputEvent>> copyEvents;
		for (auto& ev : events)
		{
			copyEvents.push_back(ev->copy());
		}
		return std::make_unique<CombinedInputEvent>(std::move(copyEvents));
	}

	std::unique_ptr<CombinedInputEvent> CombinedInputEvent::read(const nlohmann::json& json) noexcept
	{
		if (!json.is_array())
		{
			return nullptr;
		}
		std::vector<std::unique_ptr<IInputEvent>> events;
		for (auto& elm : json)
		{
			events.push_back(IInputEvent::read(elm));
		}
		return std::make_unique<CombinedInputEvent>(std::move(events));
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
		}
	}

	void InputImpl::addListener(const IInputEvent& ev, IInputEventListener& listener) noexcept
	{
		_listeners[ev.copy()] = listener;
	}

	bool InputImpl::removeListener(IInputEventListener& listener) noexcept
	{
		return false;
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
		
		EventSet lastUpdateEvents = _updateEvents;
		_updateEvents.clear();
		for (auto& elm : _listeners)
		{
			auto& ev = *elm.first;
			if (!ev.check(_input))
			{
				continue;
			}
			_updateEvents.insert(ev);
			if (!lastUpdateEvents.contains(ev))
			{
				elm.second->onInputEvent(ev);
			}
		}
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

	void Input::addListener(const IInputEvent& ev, IInputEventListener& listener) noexcept
	{
		_impl->addListener(ev, listener);
	}

	bool Input::removeListener(IInputEventListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

#pragma endregion Input

}