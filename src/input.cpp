
#include "detail/input.hpp"
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

	void KeyboardImpl::shutdown() noexcept
	{
		_listeners.clear();
	}

	void KeyboardImpl::reset() noexcept
	{
		flush();
	}

	expected<void, std::string> KeyboardImpl::setKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept
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
		std::vector<std::string> errors;
		if (changed)
		{
			for (auto& listener : _listeners)
			{
				auto result = listener.onKeyboardKey(key, modifiers, down);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> KeyboardImpl::pushChar(char32_t data) noexcept
	{
		_chars.at(_charsWrite) = data;
		_charsWrite = (_charsWrite + 1) % _chars.size();
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onKeyboardChar(data);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	bool KeyboardImpl::getKey(KeyboardKey key) const noexcept
	{
		return _keys.contains(key);
	}

	const KeyboardKeys& KeyboardImpl::getKeys() const noexcept
	{
		return _keys;
	}

	char32_t KeyboardImpl::popChar() noexcept
	{
		if (_charsRead == _charsWrite)
		{
			return {};
		}
		auto& val = _chars.at(_charsRead);
		_charsRead = (_charsRead + 1) % _chars.size();
		return val;
	}

	const KeyboardModifiers& KeyboardImpl::getModifiers() const noexcept
	{
		return _modifiers;
	}

	bool KeyboardImpl::hasModifier(KeyboardModifier mod) const noexcept
	{
		return _modifiers.contains(mod);
	}

	std::u32string_view KeyboardImpl::getUpdateChars() const noexcept
	{
		return _updateChars;
	}

	void KeyboardImpl::afterUpdate() noexcept
	{
		_updateChars.clear();
		char32_t chr;
		while (true)
		{
			chr = popChar();
			if (!chr)
			{
				break;
			}
			_updateChars.push_back(chr);
		}
	}

	void KeyboardImpl::flush() noexcept
	{
		_chars.fill({});
		_charsRead = 0;
		_charsWrite = 0;
	}

	void KeyboardImpl::addListener(std::unique_ptr<IKeyboardListener>&& listener) noexcept
	{
		_listeners.insert(std::move(listener));
	}

	void KeyboardImpl::addListener(IKeyboardListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool KeyboardImpl::removeListener(const IKeyboardListener& listener) noexcept
	{
		return _listeners.erase(listener);
	}

	size_t KeyboardImpl::removeListeners(const IKeyboardListenerFilter& filter) noexcept
	{
		return _listeners.eraseIf(filter);
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
			return '0' + char(toUnderlying(key) - toUnderlying(KeyboardKey::Key0));
		}

		const bool isChar = (KeyboardKey::KeyA <= key && key <= KeyboardKey::KeyZ);
		if (isChar)
		{
			return (upper ? 'A' : 'a') + char(toUnderlying(key) - toUnderlying(KeyboardKey::KeyA));
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

	const std::string KeyboardImpl::_keyPrefix = "KeyboardKey.";

	std::string_view KeyboardImpl::getKeyName(KeyboardKey key) noexcept
	{
		return StringUtils::getEnumName(key);
	}

	std::optional<KeyboardKey> KeyboardImpl::readKey(std::string_view name) noexcept
	{
		return StringUtils::readEnum<KeyboardKey>(name, _keyPrefix);
	}

	const std::string KeyboardImpl::_modPrefix = "KeyboardModifier.";
	
	std::string_view KeyboardImpl::getModifierName(KeyboardModifier mod) noexcept
	{
		return StringUtils::getEnumName(mod);
	}

	std::optional<KeyboardModifier> KeyboardImpl::readModifier(std::string_view name) noexcept
	{
		return StringUtils::readEnum<KeyboardModifier>(name, _modPrefix);
	}

	expected<KeyboardInputEvent, std::string> KeyboardImpl::readEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split("+", name);
		auto key = Keyboard::readKey(parts[0]);
		if (!key)
		{
			return unexpected<std::string>{ "Invalid keyboard key name: " + parts[0] };
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

	std::u32string_view Keyboard::getUpdateChars() const noexcept
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

	void Keyboard::addListener(std::unique_ptr<IKeyboardListener>&& listener) noexcept
	{
		_impl->addListener(std::move(listener));
	}

	void Keyboard::addListener(IKeyboardListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Keyboard::removeListener(const IKeyboardListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	size_t Keyboard::removeListeners(const IKeyboardListenerFilter& filter) noexcept
	{
		return _impl->removeListeners(filter);
	}

	char Keyboard::keyToAscii(KeyboardKey key, bool upper) noexcept
	{
		return KeyboardImpl::keyToAscii(key, upper);
	}

	std::string_view Keyboard::getKeyName(KeyboardKey key) noexcept
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

	std::string_view Keyboard::getModifierName(KeyboardModifier mod) noexcept
	{
		return KeyboardImpl::getModifierName(mod);
	}

#pragma endregion Keyboard

#pragma region Mouse

	MouseImpl::MouseImpl() noexcept
		: _buttons{}
		, _position(0)
		, _lastPosition(0)
		, _lastPositionTimePassed(0)
		, _velocity(0)
		, _scroll(0)
		, _active(false)
		, _hasBeenInactive(true)
	{
	}

	void MouseImpl::shutdown() noexcept
	{
		_listeners.clear();
	}

	expected<void, std::string> MouseImpl::setActive(bool active) noexcept
	{
		if (_active == active)
		{
			return {};
		}
		_active = active;
		if (!active)
		{
			_hasBeenInactive = true;
		}
		_lastPositionTimePassed = 0.F;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onMouseActive(active);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> MouseImpl::setPosition(const glm::vec2& pos) noexcept
	{
		if (_position == pos)
		{
			return {};
		}
		glm::vec2 delta(0);
		if (!_hasBeenInactive)
		{
			delta = pos - _position;
		}
		_position = pos;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onMousePositionChange(delta, _position);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> MouseImpl::setScroll(const glm::vec2& scroll) noexcept
	{
		if (scroll == glm::vec2{ 0 })
		{
			return {};
		}
		_scroll += scroll;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onMouseScrollChange(scroll, _scroll);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> MouseImpl::setButton(MouseButton button, bool down) noexcept
	{
		auto idx = toUnderlying(button);
		if (idx >= _buttons.size())
		{
			return unexpected<std::string>{"invalid button"};
		}
		if (_buttons.at(idx) == down)
		{
			return {};
		}
		_buttons.at(idx) = down;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onMouseButton(button, down);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	void MouseImpl::afterUpdate(float deltaTime) noexcept
	{
		static const float velocityInterval = 0.05F;
		if (_lastPositionTimePassed >= velocityInterval)
		{
			auto dist = _position - _lastPosition;
			_lastPosition = _position;
			_velocity = dist / _lastPositionTimePassed;
			_lastPositionTimePassed = 0.F;
		}
		else
		{
			_lastPositionTimePassed += deltaTime;
		}
		_scroll = glm::vec2(0);
		if (_active)
		{
			if (_hasBeenInactive)
			{
				_lastPosition = _position;
				_lastPositionTimePassed = 0.F;
			}
			_hasBeenInactive = false;
		}
	}

	bool MouseImpl::getButton(MouseButton button) const noexcept
	{
		auto idx = toUnderlying(button);
		if (idx >= _buttons.size())
		{
			return false;
		}
		return _buttons.at(idx);
	}

	bool MouseImpl::getActive() const noexcept
	{
		return _active;
	}

	const glm::vec2& MouseImpl::getPosition() const noexcept
	{
		return _position;
	}

	const glm::vec2& MouseImpl::getVelocity() const noexcept
	{
		return _velocity;
	}

	const glm::vec2& MouseImpl::getScroll() const noexcept
	{
		return _scroll;
	}

	const MouseButtons& MouseImpl::getButtons() const noexcept
	{
		return _buttons;
	}

	void MouseImpl::addListener(std::unique_ptr<IMouseListener>&& listener) noexcept
	{
		_listeners.insert(std::move(listener));
	}

	void MouseImpl::addListener(IMouseListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool MouseImpl::removeListener(const IMouseListener& listener) noexcept
	{
		return _listeners.erase(listener);
	}

	size_t MouseImpl::removeListeners(const IMouseListenerFilter& filter) noexcept
	{
		return _listeners.eraseIf(filter);
	}

	const std::string MouseImpl::_buttonPrefix = "MouseButton.";


	std::optional<MouseButton> MouseImpl::readButton(std::string_view name) noexcept
	{
		return StringUtils::readEnum<MouseButton>(name, _buttonPrefix);
	}

	std::string_view MouseImpl::getButtonName(MouseButton button) noexcept
	{
		return StringUtils::getEnumName(button);
	}

	const std::string MouseImpl::_analogPrefix = "MouseAnalog.";

	std::optional<MouseAnalog> MouseImpl::readAnalog(std::string_view name) noexcept
	{
		return StringUtils::readEnum<MouseAnalog>(name, _analogPrefix);
	}

	std::string_view MouseImpl::getAnalogName(MouseAnalog analog) noexcept
	{
		return StringUtils::getEnumName(analog);
	}

	std::optional<MouseInputEvent> MouseImpl::readEvent(std::string_view name) noexcept
	{
		if (auto button = readButton(name))
		{
			return MouseInputEvent{ button.value() };
		}
		return std::nullopt;
	}

	expected<MouseInputDir, std::string> MouseImpl::readDir(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(":", name);
		auto size = parts.size();
		if (size < 2)
		{
			return unexpected<std::string>{"could not find separator"};
		}
		auto analog = readAnalog(parts[0]);
		if (!analog)
		{
			return unexpected<std::string>{"could not read first part"};
		}
		auto type = InputImpl::readDirType(parts[1]);
		if (!type)
		{
			return unexpected<std::string>{"could not read second part"};
		}
		return MouseInputDir{ analog.value(), type.value() };
	}

	std::optional<MouseButton> Mouse::readButton(std::string_view name) noexcept
	{
		return MouseImpl::readButton(name);
	}

	std::string_view Mouse::getButtonName(MouseButton button) noexcept
	{
		return MouseImpl::getButtonName(button);
	}

	std::optional<MouseAnalog> Mouse::readAnalog(std::string_view name) noexcept
	{
		return MouseImpl::readAnalog(name);
	}

	std::string_view Mouse::getAnalogName(MouseAnalog analog) noexcept
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

	const glm::vec2& Mouse::getVelocity() const noexcept
	{
		return _impl->getVelocity();
	}

	const glm::vec2& Mouse::getScroll() const noexcept
	{
		return _impl->getScroll();
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

	void Mouse::addListener(std::unique_ptr<IMouseListener>&& listener) noexcept
	{
		_impl->addListener(std::move(listener));
	}

	void Mouse::addListener(IMouseListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Mouse::removeListener(const IMouseListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	size_t Mouse::removeListeners(const IMouseListenerFilter& filter) noexcept
	{
		return _impl->removeListeners(filter);
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

	void GamepadImpl::shutdown() noexcept
	{
		_listeners.clear();
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

	expected<void, std::string> GamepadImpl::setConnected(bool value) noexcept
	{
		if (_connected == value)
		{
			return {};
		}
		clear();
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onGamepadConnect(_num, value);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	void GamepadImpl::clear() noexcept
	{
		_sticks.fill(glm::ivec3(0));
		_buttons.fill(false);
	}

	const float GamepadImpl::_stickThreshold = 0.5F;

	expected<void, std::string> GamepadImpl::setStick(GamepadStick stick, const glm::vec3& value) noexcept
	{
		auto idx = toUnderlying(stick);
		if (idx >= _sticks.size())
		{
			return unexpected<std::string>{"invalid stick"};
		}
		auto& oldValue = _sticks.at(idx);
		if (oldValue == value)
		{
			return {};
		}
		_sticks.at(idx) = value;
		auto delta = value - oldValue;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onGamepadStickChange(_num, stick, delta, value);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		for (int i = 0; i < toUnderlying(InputDirType::Count); ++i)
		{
			auto dir = InputDirType(i);
			auto active = InputImpl::getDir(value, dir) >= _stickThreshold;
			if (_stickDirs[stick][dir] != active)
			{
				_stickDirs[stick][dir] = active;
				for (auto& listener : _listeners)
				{
					auto result = listener.onGamepadStickDir(_num, stick, dir, active);
					if (!result)
					{
						errors.push_back(std::move(result).error());
					}
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> GamepadImpl::setButton(GamepadButton button, bool down) noexcept
	{
		auto idx = toUnderlying(button);
		if (idx >= _buttons.size())
		{
			return unexpected<std::string>{"invalid button"};
		}
		if (_buttons.at(idx) == down)
		{
			return {};
		}
		_buttons.at(idx) = down;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onGamepadButton(_num, button, down);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	const glm::vec3& GamepadImpl::getStick(GamepadStick stick) const noexcept
	{
		auto idx = toUnderlying(stick);
		if (idx >= _sticks.size())
		{
			const static glm::vec3 zero(0);
			return zero;
		}
		return _sticks.at(idx);
	}

	bool GamepadImpl::getStickDir(GamepadStick stick, InputDirType dir) const noexcept
	{
		auto itr = _stickDirs.find(stick);
		if(itr == _stickDirs.end())
		{
			return false;
		}
		auto itr2 = itr->second.find(dir);
		return itr2 != itr->second.end() && itr2->second;
	}

	bool GamepadImpl::getButton(GamepadButton button) const noexcept
	{
		auto idx = toUnderlying(button);
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

	void GamepadImpl::addListener(std::unique_ptr<IGamepadListener>&& listener) noexcept
	{
		_listeners.insert(std::move(listener));
	}

	void GamepadImpl::addListener(IGamepadListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool GamepadImpl::removeListener(const IGamepadListener& listener) noexcept
	{
		return _listeners.erase(listener);
	}

	size_t GamepadImpl::removeListeners(const IGamepadListenerFilter& filter) noexcept
	{
		return _listeners.eraseIf(filter);
	}

	const std::string GamepadImpl::_buttonPrefix = "GamepadButton.";

	std::optional<GamepadButton> GamepadImpl::readButton(std::string_view name) noexcept
	{
		return StringUtils::readEnum<GamepadButton>(name, _buttonPrefix);
	}

	std::string_view GamepadImpl::getButtonName(GamepadButton button) noexcept
	{
		return StringUtils::getEnumName(button);
	}

	const std::string GamepadImpl::_stickPrefix = "GamepadStick.";

	std::optional<GamepadStick> GamepadImpl::readStick(std::string_view name) noexcept
	{
		return StringUtils::readEnum<GamepadStick>(name, _stickPrefix);
	}

	std::string_view GamepadImpl::getStickName(GamepadStick stick) noexcept
	{
		return StringUtils::getEnumName(stick);
	}

	std::optional<uint8_t> GamepadImpl::readNum(std::string_view name) noexcept
	{
		auto val = std::stoul(std::string(name));
		if (val > std::numeric_limits<uint8_t>::max())
		{
			return Gamepad::Any;
		}
		return static_cast<uint8_t>(val);
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

	const uint8_t Gamepad::MaxAmount = DARMOK_GAMEPAD_MAX;
	const uint8_t Gamepad::Any = 5;

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

	std::string_view Gamepad::getButtonName(GamepadButton button) noexcept
	{
		return GamepadImpl::getButtonName(button);
	}

	std::string_view Gamepad::getStickName(GamepadStick stick) noexcept
	{
		return GamepadImpl::getStickName(stick);
	}

	const glm::vec3& Gamepad::getStick(GamepadStick stick) const noexcept
	{
		return _impl->getStick(stick);
	}

	bool Gamepad::getStickDir(GamepadStick stick, InputDirType dir) const noexcept
	{
		return _impl->getStickDir(stick, dir);
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

	void Gamepad::addListener(std::unique_ptr<IGamepadListener>&& listener) noexcept
	{
		_impl->addListener(std::move(listener));
	}

	void Gamepad::addListener(IGamepadListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Gamepad::removeListener(const IGamepadListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	size_t Gamepad::removeListeners(const IGamepadListenerFilter& filter) noexcept
	{
		return _impl->removeListeners(filter);
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

	bool GamepadStickInputEvent::operator==(const GamepadStickInputEvent& other) const noexcept
	{
		return stick == other.stick && dir == other.dir && gamepad == other.gamepad;
	}

	bool MouseInputDir::operator==(const MouseInputDir& other) const noexcept
	{
		return analog == other.analog && type == other.type;
	}

	bool GamepadInputDir::operator==(const GamepadInputDir& other) const noexcept
	{
		return stick == other.stick && type == other.type && gamepad == other.gamepad;
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

	void InputImpl::shutdown() noexcept
	{
		_keyboard.getImpl().shutdown();
		_mouse.getImpl().shutdown();
		for (auto& gamepad : _gamepads)
		{
			gamepad.getImpl().shutdown();
		}
		_listeners.clear();
	}

	void InputImpl::addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener) noexcept
	{
		_listeners.push_back(ListenerData{ tag, evs, *listener, std::move(listener) });
	}

	void InputImpl::addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept
	{
		_listeners.push_back(ListenerData{ tag, evs, listener });
	}

	bool InputImpl::removeListener(const std::string& tag, const IInputEventListener& listener) noexcept
	{
		const auto* ptr = &listener;
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&tag, ptr](auto& data)
		{
			return data.tag == tag && &data.listener.get() == ptr;
		});

		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool InputImpl::removeListener(const IInputEventListener& listener) noexcept
	{
		const auto* ptr = &listener;
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [ptr](auto& data)
		{
			return &data.listener.get() == ptr;
		});
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	size_t InputImpl::removeListeners(const IInputEventListenerFilter& filter) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&filter](auto& data)
			{
				return filter(data.tag, data.listener.get());
			});
		auto count = std::distance(itr, _listeners.end());
		_listeners.erase(itr, _listeners.end());
		return count;
	}

	bool InputImpl::checkEvents(const InputEvents& evs) const noexcept
	{
		return std::ranges::any_of(evs, [this](const auto& event) {
			return checkEvent(event);
		});
	}

	bool InputImpl::checkEvent(const InputEvent& ev) const noexcept
	{
		if (const auto* kbEv = std::get_if<KeyboardInputEvent>(&ev))
		{
			if (!_keyboard.getKey(kbEv->key))
			{
				return false;
			}
			return _keyboard.getModifiers() == kbEv->modifiers;
		}
		if (const auto* mouseEv = std::get_if<MouseInputEvent>(&ev))
		{
			return _mouse.getButton(mouseEv->button);
		}
		if (const auto* gamepadEv = std::get_if<GamepadInputEvent>(&ev))
		{
			auto gamepad = getGamepad(gamepadEv->gamepad);
			if(!gamepad)
			{
				return false;
			}
			return gamepad->getButton(gamepadEv->button);
		}
		if (const auto* gamepadStickEv = std::get_if<GamepadStickInputEvent>(&ev))
		{
			auto gamepad = getGamepad(gamepadStickEv->gamepad);
			if (!gamepad)
			{
				return false;
			}
			return gamepad->getStickDir(gamepadStickEv->stick, gamepadStickEv->dir);
		}
		return false;
	}

	float InputImpl::getDir(const glm::vec2& vec, InputDirType dir) noexcept
	{
		float val = 0.F;
		switch (dir)
		{
			case InputDirType::Up:
				val = vec.y > 0.F ? vec.y : 0.F;
				break;
			case InputDirType::Down:
				val = vec.y < 0.F ? -vec.y : 0.F;
				break;
			case InputDirType::Right:
				val = vec.x > 0.F ? vec.x : 0.F;
				break;
			case InputDirType::Left:
				val = vec.x < 0.F ? -vec.x : 0.F;
				break;
			default:
				break;
		}
		return val;
	}

	// TODO: maybe should be configurable? these values work for me
	const float InputImpl::_mouseVelocityDirFactor = 0.0004F;
	const float InputImpl::_mouseScrollDirFactor = 5.F;

	float InputImpl::getDir(const InputDir& dir, const Sensitivity& sensi) const noexcept
	{
		if (const auto* mouseDir = std::get_if<MouseInputDir>(&dir))
		{
			glm::vec2 vec(0);
			if (mouseDir->analog == MouseAnalog::Scroll)
			{
				vec = _mouse.getScroll() * _mouseScrollDirFactor;
			}
			else
			{
				vec = _mouse.getVelocity() * glm::vec2(_mouseVelocityDirFactor, -_mouseVelocityDirFactor);
			}
			return getDir(vec * sensi.mouse, mouseDir->type);
		}
		if (const auto* gamepadDir = std::get_if<GamepadInputDir>(&dir))
		{
			auto gamepad = getGamepad(gamepadDir->gamepad);
			if (!gamepad)
			{
				return 0.F;
			}
			const auto& vec = gamepad->getStick(gamepadDir->stick);
			return getDir(vec * sensi.gamepad, gamepadDir->type);
		}
		if (const auto* event = std::get_if<InputEvent>(&dir))
		{
			return checkEvent(*event) ? sensi.event : 0.F;
		}
		return 0.F;
	}

	float InputImpl::getAxis(const Dirs& negative, const Dirs& positive, const Sensitivity& sensi) const noexcept
	{
		float val = 0;
		for (const auto& dir : positive)
		{
			val += getDir(dir, sensi);
		}
		for (const auto& dir : negative)
		{
			val -= getDir(dir, sensi);
		}
		return val;
	}

	expected<void, std::string> InputImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept
	{
		if (!down)
		{
			return {};
		}
		std::vector<std::string> errors;
		for (auto& data : _listeners)
		{
			for (auto& event : data.events)
			{
				auto* keyEvent = std::get_if<KeyboardInputEvent>(&event);
				if (!keyEvent)
				{
					continue;
				}
				if (keyEvent->key != key || keyEvent->modifiers != modifiers)
				{
					continue;
				}
				auto result = data.listener.get().onInputEvent(data.tag);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> InputImpl::onMouseButton(MouseButton button, bool down) noexcept
	{
		if (!down)
		{
			return {};
		}
		std::vector<std::string> errors;
		for (auto& data : _listeners)
		{
			for (auto& event : data.events)
			{
				auto* mouseEvent = std::get_if<MouseInputEvent>(&event);
				if (!mouseEvent || mouseEvent->button != button)
				{
					continue;
				}
				auto result = data.listener.get().onInputEvent(data.tag);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> InputImpl::onGamepadButton(uint8_t num, GamepadButton button, bool down) noexcept
	{
		if (!down)
		{
			return {};
		}
		std::vector<std::string> errors;
		for (auto& data : _listeners)
		{
			for (auto& event : data.events)
			{
				const auto* gamepadEvent = std::get_if<GamepadInputEvent>(&event);
				if (!gamepadEvent)
				{
					continue;
				}
				if (gamepadEvent->gamepad != num && gamepadEvent->gamepad != Gamepad::Any)
				{
					continue;
				}
				if (gamepadEvent->button != button)
				{
					continue;
				}
				auto result = data.listener.get().onInputEvent(data.tag);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> InputImpl::onGamepadStickDir(uint8_t num, GamepadStick stick, InputDirType dir, bool active) noexcept
	{
		if (!active)
		{
			return {};
		}
		std::vector<std::string> errors;
		for (auto & data : _listeners)
		{
			for (auto& event : data.events)
			{
				const auto* gamepadEvent = std::get_if<GamepadStickInputEvent>(&event);
				if (!gamepadEvent)
				{
					continue;
				}
				if (gamepadEvent->gamepad != num && gamepadEvent->gamepad != Gamepad::Any)
				{
					continue;
				}
				if (gamepadEvent->stick != stick || gamepadEvent->dir != dir)
				{
					continue;
				}
				auto result = data.listener.get().onInputEvent(data.tag);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	const std::string InputImpl::_dirTypePrefix = "InputDirType.";

	std::string_view InputImpl::getDirTypeName(InputDirType type) noexcept
	{
		return StringUtils::getEnumName(type);
	}

	std::optional<InputDirType> InputImpl::readDirType(std::string_view name) noexcept
	{
		return StringUtils::readEnum<InputDirType>(name, _dirTypePrefix);
	}

	const std::string InputImpl::_keyboardPrefix = "keyboard:";
	const std::string InputImpl::_mousePrefix = "mouse:";
	const std::string InputImpl::_gamepadPrefix = "gamepad:";

	expected<InputEvent, std::string> InputImpl::readEvent(std::string_view name) noexcept
	{
		if (name.starts_with(_keyboardPrefix))
		{
			return KeyboardImpl::readEvent(name.substr(_keyboardPrefix.size()));
		}
		if (name.starts_with(_mousePrefix))
		{
			if (auto result = MouseImpl::readEvent(name.substr(_mousePrefix.size())))
			{
				return *result;
			}
			return unexpected<std::string>{"could not read mouse event"};
		}
		if (name.starts_with(_gamepadPrefix))
		{
			if (auto result = GamepadImpl::readEvent(name.substr(_gamepadPrefix.size())))
			{
				return *result;
			}
			return unexpected<std::string>{"could not read gamepad event"};
		}
		return unexpected<std::string>{"unexpected event type"};
	}

	expected<InputDir, std::string> InputImpl::readDir(std::string_view name) noexcept
	{
		if (name.starts_with(_gamepadPrefix))
		{
			if (auto dir = GamepadImpl::readDir(name.substr(_gamepadPrefix.size())))
			{
				return *dir;
			}
			return unexpected<std::string>{"could not read gamepad dir"};
		}
		if (name.starts_with(_mousePrefix))
		{
			if (auto dir = MouseImpl::readDir(name.substr(_mousePrefix.size())))
			{
				return *dir;
			}
			return unexpected<std::string>{"could not read mouse dir"};
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
			for (const auto& gamepad : _gamepads)
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
			return _gamepads.at(num);
		}
		return nullptr;
	}

	const Gamepads& InputImpl::getGamepads() const noexcept
	{
		return _gamepads;
	}

	void InputImpl::afterUpdate(float deltaTime) noexcept
	{
		_keyboard.getImpl().afterUpdate();
		_mouse.getImpl().afterUpdate(deltaTime);
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

	void Input::addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener) noexcept
	{
		_impl->addListener(tag, evs, std::move(listener));
	}

	void Input::addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept
	{
		_impl->addListener(tag, evs, listener);
	}

	bool Input::removeListener(const IInputEventListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	bool Input::removeListener(const std::string& tag, const IInputEventListener& listener) noexcept
	{
		return _impl->removeListener(tag, listener);
	}

	size_t Input::removeListeners(const IInputEventListenerFilter& filter) noexcept
	{
		return _impl->removeListeners(filter);
	}

	expected<InputEvent, std::string> Input::readEvent(std::string_view name) noexcept
	{
		return InputImpl::readEvent(name);
	}

	expected<InputDir, std::string> Input::readDir(std::string_view name) noexcept
	{
		return InputImpl::readDir(name);
	}

	std::optional<InputDirType> Input::readDirType(std::string_view name) noexcept
	{
		return InputImpl::readDirType(name);
	}

	std::string_view Input::getDirTypeName(InputDirType type) noexcept
	{
		return InputImpl::getDirTypeName(type);
	}

	bool Input::checkEvent(const InputEvent& ev) const noexcept
	{
		return _impl->checkEvent(ev);
	}

	bool Input::checkEvents(const InputEvents& evs) const noexcept
	{
		return _impl->checkEvents(evs);
	}

	float Input::getAxis(const InputDirs& negative, const InputDirs& positive, const Sensitivity& sensi) const noexcept
	{
		return _impl->getAxis(negative, positive, sensi);
	}

	float Input::getAxis(const InputAxis& axis, const Sensitivity& sensitivity) const noexcept
	{
		return _impl->getAxis(axis.first, axis.second, sensitivity);
	}

#pragma endregion Input

}