
#include "detail/input.hpp"
#include <bx/bx.h>
#include <darmok/window.hpp>
#include <darmok/string.hpp>

namespace darmok
{
#pragma region Keyboard

	KeyboardImpl::KeyboardImpl() noexcept
		: _keys{}
		, _charsRead{ 0 }
		, _charsWrite{ 0 }
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

	bool KeyboardImpl::getModifier(KeyboardModifier mod) const noexcept
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

	void KeyboardImpl::addListener(std::unique_ptr<IKeyboardListener> listener) noexcept
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
		constexpr auto Key0 = Keyboard::Definition::Key0;
		constexpr auto Key9 = Keyboard::Definition::Key9;
		constexpr auto KeyA = Keyboard::Definition::KeyA;
		constexpr auto KeyZ = Keyboard::Definition::KeyZ;
		constexpr auto Esc = Keyboard::Definition::KeyEsc;
		constexpr auto Minus = Keyboard::Definition::KeyMinus;

		const bool isAscii = (Key0 <= key && key <= KeyZ)
			|| (Esc <= key && key <= Minus);
		if (!isAscii)
		{
			return '\0';
		}

		const bool isNumber = (Key0 <= key && key <= Key9);
		if (isNumber)
		{
			return '0' + static_cast<char>(key - Key0);
		}

		const bool isChar = (KeyA <= key && key <= KeyZ);
		if (isChar)
		{
			return (upper ? 'A' : 'a') + static_cast<char>(key - KeyA);
		}

		switch (key)
		{
		case Keyboard::Definition::KeyEsc:       return 0x1b;
		case Keyboard::Definition::KeyReturn:    return '\n';
		case Keyboard::Definition::KeyTab:       return '\t';
		case Keyboard::Definition::KeySpace:     return ' ';
		case Keyboard::Definition::KeyBackspace: return 0x08;
		case Keyboard::Definition::KeyPlus:      return '+';
		case Keyboard::Definition::KeyMinus:     return '-';
		default:
			break;
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
		return StringUtils::readEnum<KeyboardKey>(name, _keyPrefix, "Keyboard_Key_Key");
	}

	const std::string KeyboardImpl::_modPrefix = "KeyboardModifier.";
	
	std::string_view KeyboardImpl::getModifierName(KeyboardModifier mod) noexcept
	{
		return StringUtils::getEnumName(mod);
	}

	std::optional<KeyboardModifier> KeyboardImpl::readModifier(std::string_view name) noexcept
	{
		return StringUtils::readEnum<KeyboardModifier>(name, _modPrefix, "Keyboard_Modifier_Modifier");
	}

	expected<KeyboardInputEvent, std::string> KeyboardImpl::readEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split('+', name);
		auto key = Keyboard::readKey(parts[0]);
		if (!key)
		{
			return unexpected<std::string>{ "Invalid keyboard key name: " + parts[0] };
		}
		KeyboardInputEvent ev;
		ev.set_key(key.value());
		for (size_t i = 1; i < parts.size(); i++)
		{
			auto mod = Keyboard::readModifier(parts[i]);
			if (mod)
			{
				ev.mutable_modifiers()->Add(mod.value());
			}
		}
		return ev;
	}

	KeyboardModifiers KeyboardImpl::convertModifiers(const google::protobuf::RepeatedField<int>& field) noexcept
	{
		KeyboardModifiers mods;
		mods.reserve(field.size());
		for (auto mod : field)
		{
			mods.emplace(static_cast<KeyboardModifier>(mod));
		}
		return mods;
	}

	Keyboard::Keyboard() noexcept
		: _impl(std::make_unique<KeyboardImpl>())
	{
	}

	Keyboard::~Keyboard() noexcept = default;

	bool Keyboard::getKey(KeyboardKey key) const noexcept
	{
		return _impl->getKey(key);
	}

	bool Keyboard::getModifier(KeyboardModifier mod) const noexcept
	{
		return _impl->getModifier(mod);
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

	void Keyboard::addListener(std::unique_ptr<IKeyboardListener> listener) noexcept
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

	InputEvent Keyboard::createInputEvent(KeyboardKey key) noexcept
	{
		InputEvent ev;
		ev.mutable_keyboard()->set_key(key);
		return ev;
	}

	InputDir Keyboard::createInputDir(KeyboardKey key) noexcept
	{
		InputDir dir;
		*dir.mutable_event() = createInputEvent(key);
		return dir;
	}

#pragma endregion Keyboard

#pragma region Mouse

	MouseImpl::MouseImpl() noexcept
		: _buttons{}
		, _position{ 0 }
		, _lastPosition{ 0 }
		, _lastPositionTimePassed{ 0 }
		, _velocity{ 0 }
		, _scroll{ 0 }
		, _active{ false }
		, _hasBeenInactive{ true }
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
		auto old = getButton(button);
		if (old == down)
		{
			return {};
		}
		_buttons[button] = down;
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
		auto itr = _buttons.find(button);
		if (itr == _buttons.end())
		{
			return false;
		}
		return itr->second;
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

	void MouseImpl::addListener(std::unique_ptr<IMouseListener> listener) noexcept
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
		return StringUtils::readEnum<MouseButton>(name, _buttonPrefix, "Mouse_Button_Button");
	}

	std::string_view MouseImpl::getButtonName(MouseButton button) noexcept
	{
		return StringUtils::getEnumName(button);
	}

	const std::string MouseImpl::_analogPrefix = "MouseAnalog.";

	std::optional<MouseAnalog> MouseImpl::readAnalog(std::string_view name) noexcept
	{
		return StringUtils::readEnum<MouseAnalog>(name, _analogPrefix, "Mouse_Analog_Analog");
	}

	std::string_view MouseImpl::getAnalogName(MouseAnalog analog) noexcept
	{
		return StringUtils::getEnumName(analog);
	}

	expected<MouseInputEvent, std::string> MouseImpl::readEvent(std::string_view name) noexcept
	{
		auto button = readButton(name);
		if (!button)
		{
			return unexpected<std::string>{"could not read button"};
		}
		MouseInputEvent ev;
		ev.set_button(button.value());
		return ev;
	}

	expected<MouseInputDir, std::string> MouseImpl::readDir(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(':', name);
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
		MouseInputDir dir;
		dir.set_analog(analog.value());
		dir.set_type(type.value());
		return dir;
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

	InputEvent Mouse::createInputEvent(MouseButton button) noexcept
	{
		InputEvent ev;
		ev.mutable_mouse()->set_button(button);
		return ev;
	}

	InputDir Mouse::createInputDir(InputDirType dirType, MouseAnalog analog) noexcept
	{
		InputDir dir;
		auto& mouse = *dir.mutable_mouse();
		mouse.set_analog(analog);
		mouse.set_type(dirType);
		return dir;
	}

	Mouse::Mouse() noexcept
		: _impl(std::make_unique<MouseImpl>())
	{
	}

	Mouse::~Mouse() noexcept = default;

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

	void Mouse::addListener(std::unique_ptr<IMouseListener> listener) noexcept
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

	GamepadImpl::GamepadImpl(uint32_t num) noexcept
		: _num{ num }
		, _connected{ false }
		, _sticks{}
		, _buttons{}
	{
	}

	void GamepadImpl::shutdown() noexcept
	{
		_listeners.clear();
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
		_sticks.clear();
		_buttons.clear();
	}

	const float GamepadImpl::_stickThreshold = 0.5F;

	expected<void, std::string> GamepadImpl::setStick(GamepadStick stick, const glm::vec3& value) noexcept
	{
		auto old = getStick(stick);
		if (old == value)
		{
			return {};
		}
		_sticks[stick] = value;
		auto delta = value - old;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onGamepadStickChange(_num, stick, delta, value);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		for (auto dir : magic_enum::enum_values<InputDirType>())
		{
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
		auto old = getButton(button);
		if (old == down)
		{
			return {};
		}
		_buttons[button] = down;
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
		auto itr = _sticks.find(stick);
		if (itr == _sticks.end())
		{
			const static glm::vec3 zero{ 0 };
			return zero;
		}
		return itr->second;
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
		auto itr = _buttons.find(button);
		if (itr == _buttons.end())
		{
			return false;
		}
		return itr->second;
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
		return _num >= 0;
	}

	void GamepadImpl::addListener(std::unique_ptr<IGamepadListener> listener) noexcept
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
		return StringUtils::readEnum<GamepadButton>(name, _buttonPrefix, "Gamepad_Button_Button");
	}

	std::string_view GamepadImpl::getButtonName(GamepadButton button) noexcept
	{
		return StringUtils::getEnumName(button);
	}

	const std::string GamepadImpl::_stickPrefix = "GamepadStick.";

	std::optional<GamepadStick> GamepadImpl::readStick(std::string_view name) noexcept
	{
		return StringUtils::readEnum<GamepadStick>(name, _stickPrefix, "Gamepad_Stick_Stick");
	}

	std::string_view GamepadImpl::getStickName(GamepadStick stick) noexcept
	{
		return StringUtils::getEnumName(stick);
	}

	std::optional<uint32_t> GamepadImpl::readNum(std::string_view name) noexcept
	{
		auto val = std::stoul(std::string(name));
		if (val > std::numeric_limits<uint32_t>::max())
		{
			return -1;
		}
		return static_cast<uint32_t>(val);
	}

	expected<GamepadInputEvent, std::string> GamepadImpl::readEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(':', name);
		auto size = parts.size();
		if (size != 2)
		{
			return unexpected<std::string>{"missing parts"};
		}
		auto button = readButton(parts[0]);
		if (!button)
		{
			return unexpected<std::string>{"could not read button"};
		}
		auto num = readNum(parts[1]).value_or(-1);
		GamepadInputEvent ev;
		ev.set_button(button.value());
		ev.set_gamepad(num);
		return ev;
	}

	expected<GamepadStickInputEvent, std::string> GamepadImpl::readStickEvent(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(':', name);
		auto size = parts.size();
		if (size != 3)
		{
			return unexpected<std::string>{"missing parts"};
		}
		auto stick = readStick(parts[0]);
		if (!stick)
		{
			return unexpected<std::string>{"could not read stick"};
		}
		auto dir = InputImpl::readDirType(parts[1]);
		if (!dir)
		{
			return unexpected<std::string>{"could not read dir"};
		}
		auto num = readNum(parts[2]).value_or(-1);
		GamepadStickInputEvent ev;
		ev.set_stick(stick.value());
		ev.set_dir(dir.value());
		ev.set_gamepad(num);
		return ev;
	}

	expected<GamepadInputDir, std::string> GamepadImpl::readDir(std::string_view name) noexcept
	{
		auto parts = StringUtils::split(':', name);
		auto size = parts.size();
		if (size < 2)
		{
			return unexpected<std::string>{"missing parts"};
		}
		auto stick = readStick(parts[0]);
		if (!stick)
		{
			return unexpected<std::string>{"could not read stick"};
		}
		auto type = InputImpl::readDirType(parts[1]);
		if (!type)
		{
			return unexpected<std::string>{"could not read dir type"};
		}
		auto gamepad = readNum(parts[1]).value_or(-1);
		GamepadInputDir dir;
		dir.set_stick(stick.value());
		dir.set_type(type.value());
		dir.set_gamepad(gamepad);
		return dir;
	}

	Gamepad::Gamepad(uint32_t num) noexcept
		: _impl(std::make_unique<GamepadImpl>(num))
	{
	}

	Gamepad::~Gamepad() noexcept = default;

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

	void Gamepad::addListener(std::unique_ptr<IGamepadListener> listener) noexcept
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

	InputEvent Gamepad::createInputEvent(GamepadButton button, uint32_t gamepad) noexcept
	{
		InputEvent ev;
		auto& gp = *ev.mutable_gamepad();
		gp.set_button(button);
		gp.set_gamepad(gamepad);
		return ev;
	}

	InputEvent Gamepad::createInputEvent(InputDirType dirType, GamepadStick stick, uint32_t gamepad) noexcept
	{
		InputEvent ev;
		auto& gp = *ev.mutable_gamepad_stick();
		gp.set_stick(stick);
		gp.set_dir(dirType);
		gp.set_gamepad(gamepad);
		return ev;
	}

	InputDir Gamepad::createInputDir(InputDirType dirType, GamepadStick stick, uint32_t gamepad) noexcept
	{
		InputDir dir;
		auto& gpDir = *dir.mutable_gamepad();
		gpDir.set_stick(stick);
		gpDir.set_type(dirType);
		gpDir.set_gamepad(gamepad);
		return dir;
	}

#pragma endregion Gamepad

#pragma region Input

	InputImpl::InputImpl(Input& input) noexcept
		: _input(input)
	{
		_keyboard.addListener(*this);
		_mouse.addListener(*this);
	}

	void InputImpl::shutdown() noexcept
	{
		_keyboard.getImpl().shutdown();
		_mouse.getImpl().shutdown();
		for (auto& [num, gamepad] : _gamepads)
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
		if (ev.has_keyboard())
		{
			auto& kbEv = ev.keyboard();
			if (!_keyboard.getKey(kbEv.key()))
			{
				return false;
			}
			return _keyboard.getModifiers() == KeyboardImpl::convertModifiers(kbEv.modifiers());
		}
		if (ev.has_mouse())
		{
			return _mouse.getButton(ev.mouse().button());
		}
		if (ev.has_gamepad())
		{
			auto& gpEv = ev.gamepad();
			auto gamepad = getGamepad(gpEv.gamepad());
			if(!gamepad)
			{
				return false;
			}
			return gamepad->getButton(gpEv.button());
		}
		if (ev.has_gamepad_stick())
		{
			auto& gpEv = ev.gamepad_stick();
			auto gamepad = getGamepad(gpEv.gamepad());
			if (!gamepad)
			{
				return false;
			}
			return gamepad->getStickDir(gpEv.stick(), gpEv.dir());
		}
		return false;
	}

	float InputImpl::getDir(const glm::vec2& vec, InputDirType dir) noexcept
	{
		float val = 0.F;
		switch (dir)
		{
			case Input::Definition::DirUp:
				val = vec.y > 0.F ? vec.y : 0.F;
				break;
			case Input::Definition::DirDown:
				val = vec.y < 0.F ? -vec.y : 0.F;
				break;
			case Input::Definition::DirRight:
				val = vec.x > 0.F ? vec.x : 0.F;
				break;
			case Input::Definition::DirLeft:
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
		if (dir.has_mouse())
		{
			glm::vec2 vec{ 0 };
			auto& mouseDir = dir.mouse();
			if (mouseDir.analog() == Mouse::Definition::AnalogScroll)
			{
				vec = _mouse.getScroll() * _mouseScrollDirFactor;
			}
			else
			{
				vec = _mouse.getVelocity() * glm::vec2{ _mouseVelocityDirFactor, -_mouseVelocityDirFactor };
			}
			return getDir(vec * sensi.mouse, mouseDir.type());
		}
		if (dir.has_gamepad())
		{
			auto& gpDir = dir.gamepad();
			auto gamepad = getGamepad(gpDir.gamepad());
			if (!gamepad)
			{
				return 0.F;
			}
			const auto& vec = gamepad->getStick(gpDir.stick());
			return getDir(vec * sensi.gamepad, gpDir.type());
		}
		if (dir.has_event())
		{
			return checkEvent(dir.event()) ? sensi.event : 0.F;
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
				if (!event.has_keyboard())
				{
					continue;
				}
				auto& keyEvent = event.keyboard();
				if (keyEvent.key() != key || modifiers != KeyboardImpl::convertModifiers(keyEvent.modifiers()))
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
				if (!event.has_mouse())
				{
					continue;
				}
				if (event.mouse().button() != button)
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
				if (!event.has_gamepad())
				{
					continue;
				}
				auto& gpEvent = event.gamepad();
				if (gpEvent.gamepad() != num && gpEvent.gamepad() != Gamepad::Any)
				{
					continue;
				}
				if (gpEvent.button() != button)
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
				if (!event.has_gamepad_stick())
				{
					continue;
				}
				auto& gpEvent = event.gamepad_stick();
				if (gpEvent.gamepad() != num && gpEvent.gamepad() != Gamepad::Any)
				{
					continue;
				}
				if (gpEvent.stick() != stick || gpEvent.dir() != dir)
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
		return StringUtils::readEnum<InputDirType>(name, _dirTypePrefix, "Input_DirType_Dir");
	}

	const std::string InputImpl::_keyboardPrefix = "keyboard:";
	const std::string InputImpl::_mousePrefix = "mouse:";
	const std::string InputImpl::_gamepadPrefix = "gamepad:";
	const std::string InputImpl::_gamepadStickPrefix = "gamepad_stick:";

	expected<InputEvent, std::string> InputImpl::readEvent(std::string_view name) noexcept
	{
		if (name.starts_with(_keyboardPrefix))
		{
			auto result = KeyboardImpl::readEvent(name.substr(_keyboardPrefix.size()));
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
			InputEvent ev;
			*ev.mutable_keyboard() = std::move(result).value();
			return ev;
		}
		if (name.starts_with(_mousePrefix))
		{
			auto result = MouseImpl::readEvent(name.substr(_mousePrefix.size()));
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
			InputEvent ev;
			*ev.mutable_mouse() = std::move(result).value();
			return ev;
		}
		if (name.starts_with(_gamepadPrefix))
		{
			auto result = GamepadImpl::readEvent(name.substr(_gamepadPrefix.size()));
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
			InputEvent ev;
			*ev.mutable_gamepad() = std::move(result).value();
			return ev;
		}
		if (name.starts_with(_gamepadStickPrefix))
		{
			auto result = GamepadImpl::readStickEvent(name.substr(_gamepadStickPrefix.size()));
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
			InputEvent ev;
			*ev.mutable_gamepad_stick() = std::move(result).value();
			return ev;
		}
		return unexpected<std::string>{"unexpected event type"};
	}

	expected<InputDir, std::string> InputImpl::readDir(std::string_view name) noexcept
	{
		if (name.starts_with(_gamepadPrefix))
		{
			auto gpDir = GamepadImpl::readDir(name.substr(_gamepadPrefix.size()));
			if (!gpDir)
			{
				return unexpected{ std::move(gpDir).error() };
			}
			InputDir dir;
			*dir.mutable_gamepad() = std::move(gpDir).value();
			return dir;
		}
		if (name.starts_with(_mousePrefix))
		{
			auto mouseDir = MouseImpl::readDir(name.substr(_mousePrefix.size()));
			if (!mouseDir)
			{
				return unexpected{ std::move(mouseDir).error() };
			}
			InputDir dir;
			*dir.mutable_mouse() = std::move(mouseDir).value();
			return dir;
		}
		auto ev = readEvent(name);
		if (!ev)
		{
			return unexpected{ std::move(ev).error() };
		}
		InputDir dir;
		*dir.mutable_event() = std::move(ev).value();
		return dir;
	}

	Keyboard& InputImpl::getKeyboard() noexcept
	{
		return _keyboard;
	}

	Mouse& InputImpl::getMouse() noexcept
	{
		return _mouse;
	}

	const Keyboard& InputImpl::getKeyboard() const noexcept
	{
		return _keyboard;
	}

	const Mouse& InputImpl::getMouse() const noexcept
	{
		return _mouse;
	}

	std::vector<uint32_t> InputImpl::getGamepadNums() const noexcept
	{
		std::vector<uint32_t> nums;
		nums.reserve(_gamepads.size());
		for (const auto& [num, gamepad] : _gamepads)
		{
			nums.push_back(num);
		}
		return nums;
	}

	Gamepad& InputImpl::getGamepad(uint32_t num) noexcept
	{
		if (num == Gamepad::Any)
		{
			auto gp = getConnectedGamepad();
			if (gp)
			{
				return *gp;
			}
			if (_gamepads.empty())
			{
				_gamepads.emplace(0, 0);
			}
			return _gamepads.begin()->second;
		}
		auto itr = _gamepads.find(num);
		if (itr != _gamepads.end())
		{
			return itr->second;
		}

		auto& gamepad = _gamepads.emplace(num, num).first->second;
		gamepad.addListener(*this);
		return gamepad;
	}

	OptionalRef<Gamepad> InputImpl::getConnectedGamepad() noexcept
	{
		for (auto& [num, gamepad] : _gamepads)
		{
			if (gamepad.isConnected())
			{
				return gamepad;
			}
		}
		return nullptr;
	}

	OptionalRef<const Gamepad> InputImpl::getGamepad(uint32_t num) const noexcept
	{
		if (num == Gamepad::Any)
		{
			return getConnectedGamepad();
		}
		auto itr = _gamepads.find(num);
		if (itr != _gamepads.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const Gamepad> InputImpl::getConnectedGamepad() const noexcept
	{
		for (auto& [num, gamepad] : _gamepads)
		{
			if (gamepad.isConnected())
			{
				return gamepad;
			}
		}
		return nullptr;
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

	Input::~Input() noexcept = default;

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

	Gamepad& Input::getGamepad(uint32_t num) noexcept
	{
		return _impl->getGamepad(num);
	}

	OptionalRef<const Gamepad> Input::getGamepad(uint32_t num) const noexcept
	{
		return _impl->getGamepad(num);
	}

	OptionalRef<Gamepad> Input::getConnectedGamepad() noexcept
	{
		return _impl->getConnectedGamepad();
	}

	OptionalRef<const Gamepad> Input::getConnectedGamepad() const noexcept
	{
		return _impl->getConnectedGamepad();
	}

	const InputImpl& Input::getImpl() const noexcept
	{
		return *_impl;
	}

	InputImpl& Input::getImpl() noexcept
	{
		return *_impl;
	}

	void Input::addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener> listener) noexcept
	{
		_impl->addListener(tag, evs, std::move(listener));
	}

	void Input::addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept
	{
		_impl->addListener(tag, evs, listener);
	}

	void Input::addListener(const std::string& tag, const google::protobuf::RepeatedPtrField<InputEvent>& evs, std::unique_ptr<IInputEventListener> listener) noexcept
	{
		std::vector<InputEvent> events{ evs.begin(), evs.end() };
		return addListener(tag, events, std::move(listener));
	}

	void Input::addListener(const std::string& tag, const google::protobuf::RepeatedPtrField<InputEvent>& evs, IInputEventListener& listener) noexcept
	{
		std::vector<InputEvent> events{ evs.begin(), evs.end() };
		return addListener(tag, events, listener);
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

	Input::MoveDirsDefinition Input::createMoveDirsDefinition() noexcept
	{
		MoveDirsDefinition def;
		auto add = [](auto field, KeyboardKey key1, KeyboardKey key2, InputDirType gamepadDir)
		{
			field->Add(Keyboard::createInputDir(key1));
			field->Add(Keyboard::createInputDir(key2));
			field->Add(Gamepad::createInputDir(gamepadDir));
		};
		add(def.mutable_left(), Keyboard::Definition::KeyLeft, Keyboard::Definition::KeyA, Definition::DirLeft);
		add(def.mutable_right(), Keyboard::Definition::KeyRight, Keyboard::Definition::KeyD, Definition::DirRight);
		add(def.mutable_forward(), Keyboard::Definition::KeyUp, Keyboard::Definition::KeyW, Definition::DirUp);
		add(def.mutable_backward(), Keyboard::Definition::KeyDown, Keyboard::Definition::KeyS, Definition::DirDown);
		return def;
	}

	Input::LookDirsDefinition Input::createLookDirsDefinition() noexcept
	{
		LookDirsDefinition def;
		auto add = [](auto field, InputDirType dir)
		{
			field->Add(Mouse::createInputDir(dir));
			field->Add(Gamepad::createInputDir(dir));
		};
		add(def.mutable_left(), Definition::DirLeft);
		add(def.mutable_right(), Definition::DirRight);
		add(def.mutable_up(), Definition::DirUp);
		add(def.mutable_down(), Definition::DirDown);
		return def;
	}

	glm::vec2 Input::getMoveDir(const MoveDirsDefinition& def) const noexcept
	{
		return {
			getAxis(def.left(), def.right()),
			getAxis(def.backward(), def.forward())
		};
	}

	glm::vec2 Input::getLookDir(const LookDirsDefinition& def) const noexcept
	{
		return {
			getAxis(def.left(), def.right()),
			getAxis(def.down(), def.up())
		};
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

	float Input::getAxis(const google::protobuf::RepeatedPtrField<InputDir>& negative, const google::protobuf::RepeatedPtrField<InputDir>& positive, const Sensitivity& sensitivity) const noexcept
	{
		std::vector<InputDir> negVec{ negative.begin(), negative.end() };
		std::vector<InputDir> posVec{ positive.begin(), positive.end() };
		return getAxis(negVec, posVec, sensitivity);
	}

#pragma endregion Input

}