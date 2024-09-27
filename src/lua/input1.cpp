#include "input.hpp"
#include "window.hpp"
#include "utils.hpp"
#include <darmok/input.hpp>
#include <darmok/string.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>

namespace darmok
{
	LuaKeyboard::LuaKeyboard(Keyboard& kb) noexcept
		: _kb(kb)
	{
		_kb.get().addListener(*this);
	}

	LuaKeyboard::~LuaKeyboard() noexcept
	{
		_kb.get().removeListener(*this);
	}

	const LuaTableDelegateDefinition LuaKeyboard::_keyDelegate("on_keyboard_key", "running keyboard key listener");
	const LuaTableDelegateDefinition LuaKeyboard::_charDelegate("on_keyboard_char", "running keyboard on_keyboard_char listener");

	void LuaKeyboard::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down)
	{
		_keyDelegate(_listeners, modifiers, down);
	}

	void LuaKeyboard::onKeyboardChar(const Utf8Char& chr)
	{
		auto str = chr.toString();
		_charDelegate(_listeners, str);
	}

	std::string LuaKeyboard::getUpdateChars() const noexcept
	{
		std::stringstream ss;
		for (auto& chr : _kb.get().getUpdateChars())
		{
			ss << chr;
		}
		return ss.str();
	}

	void LuaKeyboard::addListener(const sol::table& table) noexcept
	{
		_listeners.push_back(table);
	}


	bool LuaKeyboard::removeListener(const sol::table& table) noexcept
	{
		auto itr = std::remove(_listeners.begin(), _listeners.end(), table);
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool LuaKeyboard::getKey(KeyboardKey key) const noexcept
	{
		return _kb.get().getKey(key);
	}

	const KeyboardKeys& LuaKeyboard::getKeys() const noexcept
	{
		return _kb.get().getKeys();
	}

	const KeyboardModifiers& LuaKeyboard::getModifiers() const noexcept
	{
		return _kb.get().getModifiers();
	}


	std::optional<KeyboardKey> LuaKeyboard::readKey(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Keyboard::readKey(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<KeyboardModifier> LuaKeyboard::readModifier(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Keyboard::readModifier(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<KeyboardInputEvent> LuaKeyboard::readEvent(const sol::object& val) noexcept
	{
		if (val.is<KeyboardInputEvent>())
		{
			return val.as<KeyboardInputEvent>();
		}
		if (auto key = readKey(val))
		{
			return KeyboardInputEvent{ key.value() };
		}
		if (!val.is<sol::table>())
		{
			return std::nullopt;
		}
		auto tab = val.as<sol::table>();
		auto size = tab.size();
		if (size == 0)
		{
			return std::nullopt;
		}
		auto key = readKey(tab[1]);
		if (!key)
		{
			return std::nullopt;
		}
		KeyboardModifiers mods;
		for (size_t i = 2; i <= size; i++)
		{
			if (auto mod = readModifier(tab[i]))
			{
				mods.insert(mod.value());
			}
		}
		return KeyboardInputEvent{ key.value(), mods };
	}

	void LuaKeyboard::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnumFunc(lua, "KeyboardKey", KeyboardKey::Count, &Keyboard::getKeyName, true);
		LuaUtils::newEnumFunc(lua, "KeyboardModifier", KeyboardModifier::Count, Keyboard::getModifierName, true);

		lua.new_usertype<KeyboardInputEvent>("KeyboardInputEvent", sol::default_constructor,
			"key", &KeyboardInputEvent::key,
			"modifiers", &KeyboardInputEvent::modifiers
		);

		lua.new_usertype<LuaKeyboard>("Keyboard", sol::no_constructor,
			"get_key", &LuaKeyboard::getKey,
			"chars", sol::property(&LuaKeyboard::getUpdateChars),
			"add_listener", &LuaKeyboard::addListener,
			"remove_listener", &LuaKeyboard::removeListener
		);
	}

	LuaMouse::LuaMouse(Mouse& mouse) noexcept
		: _mouse(mouse)
	{
		_mouse.get().addListener(*this);
	}

	LuaMouse::~LuaMouse() noexcept
	{
		_mouse.get().removeListener(*this);
	}

	bool LuaMouse::getActive() const noexcept
	{
		return _mouse.get().getActive();
	}

	const glm::vec2& LuaMouse::getPosition() const noexcept
	{
		return _mouse.get().getPosition();
	}

	const glm::vec2& LuaMouse::getVelocity() const noexcept
	{
		return _mouse.get().getVelocity();
	}

	const glm::vec2& LuaMouse::getScroll() const noexcept
	{
		return _mouse.get().getScroll();
	}

	bool LuaMouse::getButton(MouseButton button) const noexcept
	{
		return _mouse.get().getButton(button);
	}

	bool LuaMouse::getLeftButton() const noexcept
	{
		return _mouse.get().getButton(MouseButton::Left);
	}

	bool LuaMouse::getMiddleButton() const noexcept
	{
		return _mouse.get().getButton(MouseButton::Middle);
	}

	bool LuaMouse::getRightButton() const noexcept
	{
		return _mouse.get().getButton(MouseButton::Right);
	}

	const LuaTableDelegateDefinition LuaMouse::_posDelegate("on_mouse_position_change", "running mouse position change listener");
	const LuaTableDelegateDefinition LuaMouse::_scrollDelegate("on_mouse_scroll_change", "running mouse scroll change listener");
	const LuaTableDelegateDefinition LuaMouse::_buttonDelegate("on_mouse_button", "running mouse button listener");

	void LuaMouse::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		_posDelegate(_listeners, delta, absolute);
	}

	void LuaMouse::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		_scrollDelegate(_listeners, delta, absolute);
	}

	void LuaMouse::onMouseButton(MouseButton button, bool down)
	{
		_buttonDelegate(_listeners, button, down);
	}

	bool LuaMouse::removeListener(const sol::table& table) noexcept
	{
		auto itr = std::remove(_listeners.begin(), _listeners.end(), table);
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	void LuaMouse::addListener(const sol::table& table) noexcept
	{
		_listeners.push_back(table);
	}

	std::optional<MouseButton> LuaMouse::readButton(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Mouse::readButton(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<MouseInputEvent> LuaMouse::readEvent(const sol::object& val) noexcept
	{
		if (val.is<MouseInputEvent>())
		{
			return val.as<MouseInputEvent>();
		}
		auto button = readButton(val);
		if (button)
		{
			return MouseInputEvent{ button.value() };
		}
		return std::nullopt;
	}

	std::optional<MouseInputDir> LuaMouse::readDir(const sol::object& val) noexcept
	{
		if (val.is<MouseInputDir>())
		{
			return val.as<MouseInputDir>();
		}
		if (!val.is<sol::table>())
		{
			return std::nullopt;
		}
		auto tab = val.as<sol::table>();
		auto size = tab.size();
		if (size < 2)
		{
			return std::nullopt;
		}
		auto analog = readAnalog(tab[1]);
		if (!analog)
		{
			return std::nullopt;
		}
		auto dirType = LuaInput::readDirType(tab[2]);
		if (!dirType)
		{
			return std::nullopt;
		}
		return MouseInputDir{ analog.value(), dirType.value() };
	}

	std::optional<MouseAnalog> LuaMouse::readAnalog(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Mouse::readAnalog(val.as<std::string>());
		}
		return std::nullopt;
	}

	void LuaMouse::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnumFunc(lua, "MouseAnalog", MouseAnalog::Count, &Mouse::getAnalogName, true);
		LuaUtils::newEnumFunc(lua, "MouseButton", MouseButton::Count, &Mouse::getButtonName, true);

		lua.new_usertype<MouseInputEvent>("MouseInputEvent", sol::default_constructor,
			"key", &MouseInputEvent::button
		);

		lua.new_usertype<MouseInputDir>("MouseInputDir", sol::default_constructor,
			"analog", &MouseInputDir::analog,
			"type", &MouseInputDir::type
		);
		
		lua.new_usertype<LuaMouse>("Mouse", sol::no_constructor,
			"active", sol::property(&LuaMouse::getActive),
			"position", sol::property(&LuaMouse::getPosition),
			"velocity", sol::property(&LuaMouse::getVelocity),
			"scroll", sol::property(&LuaMouse::getScroll),
			"left_button", sol::property(&LuaMouse::getLeftButton),
			"middle_button", sol::property(&LuaMouse::getMiddleButton),
			"right_button", sol::property(&LuaMouse::getRightButton),
			"get_button", &LuaMouse::getButton,
			"add_listener", &LuaMouse::addListener,
			"remove_listener", &LuaMouse::removeListener
		);
	}

	LuaGamepad::LuaGamepad(Gamepad& gamepad) noexcept
		: _gamepad(gamepad)
	{
		_gamepad.get().addListener(*this);
	}

	LuaGamepad::~LuaGamepad() noexcept
	{
		_gamepad.get().removeListener(*this);
	}

	const LuaTableDelegateDefinition LuaGamepad::_stickDelegate("on_gamepad_stick_change", "running gamepad stick change listener");
	const LuaTableDelegateDefinition LuaGamepad::_buttonDelegate("on_gamepad_button", "running gamepad button listener");
	const LuaTableDelegateDefinition LuaGamepad::_connectDelegate("on_gamepad_connect", "running gamepad connect listener");

	void LuaGamepad::onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute)
	{
		_stickDelegate(_listeners, num, stick, delta, absolute);
	}

	void LuaGamepad::onGamepadButton(uint8_t num, GamepadButton button, bool down)
	{
		_buttonDelegate(_listeners, num, button, down);
	}

	void LuaGamepad::onGamepadConnect(uint8_t num, bool connected)
	{
		_connectDelegate(_listeners, num, connected);
	}

	void LuaGamepad::addListener(const sol::table& table) noexcept
	{
		_listeners.push_back(table);
	}

	bool LuaGamepad::removeListener(const sol::table& table) noexcept
	{
		auto itr = std::remove(_listeners.begin(), _listeners.end(), table);
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool LuaGamepad::getButton(GamepadButton button) const noexcept
	{
		return _gamepad.get().getButton(button);
	}

	const glm::vec3& LuaGamepad::getLeftStick() const noexcept
	{
		return _gamepad.get().getStick(GamepadStick::Left);
	}

	const glm::vec3& LuaGamepad::getRightStick() const noexcept
	{
		return _gamepad.get().getStick(GamepadStick::Right);
	}

	bool LuaGamepad::isConnected() const noexcept
	{
		return _gamepad.get().isConnected();
	}

	std::optional<GamepadButton> LuaGamepad::readButton(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Gamepad::readButton(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<GamepadInputEvent> LuaGamepad::readEvent(const sol::object& val) noexcept
	{
		if (val.is<GamepadInputEvent>())
		{
			return val.as<GamepadInputEvent>();
		}
		if (auto button = readButton(val))
		{
			return GamepadInputEvent{ button.value() };
		}
		if (!val.is<sol::table>())
		{
			return std::nullopt;
		}
		auto tab = val.as<sol::table>();
		auto size = tab.size();
		if (size == 0)
		{
			return std::nullopt;
		}
		auto button = readButton(tab[1]);
		if (!button)
		{
			return std::nullopt;
		}
		auto gamepad = Gamepad::Any;
		if (size > 1 && tab[2].is<int>())
		{
			gamepad = tab[2];
		}
		return GamepadInputEvent{ button.value(), gamepad };
	}

	std::optional<GamepadStick> LuaGamepad::readStick(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Gamepad::readStick(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<GamepadInputDir> LuaGamepad::readDir(const sol::object& val) noexcept
	{
		if (val.is<GamepadInputDir>())
		{
			return val.as<GamepadInputDir>();
		}
		if (!val.is<sol::table>())
		{
			return std::nullopt;
		}
		auto tab = val.as<sol::table>();
		auto size = tab.size();
		if (size < 2)
		{
			return std::nullopt;
		}
		auto stick = readStick(tab[1]);
		if (!stick)
		{
			return std::nullopt;
		}
		auto dirType = LuaInput::readDirType(tab[2]);
		if (!dirType)
		{
			return std::nullopt;
		}
		uint8_t gamepad = Gamepad::Any;
		if (size > 2 && tab[3].is<int>())
		{
			gamepad = tab[3];
		}
		return GamepadInputDir{ stick.value(), dirType.value(), gamepad };
	}

	void LuaGamepad::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnumFunc(lua, "GamepadButton", GamepadButton::Count, &Gamepad::getButtonName, true);
		LuaUtils::newEnumFunc(lua, "GamepadStick", GamepadStick::Count, &Gamepad::getStickName, true);

		lua.new_usertype<GamepadInputEvent>("GamepadInputEvent", sol::default_constructor,
			"key", &GamepadInputEvent::button,
			"gamepad", &GamepadInputEvent::gamepad
		);

		lua.new_usertype<GamepadInputDir>("GamepadInputDir", sol::default_constructor,
			"stick", &GamepadInputDir::stick,
			"type", &GamepadInputDir::type,
			"gamepad", &GamepadInputDir::gamepad
		);

		lua.new_usertype<LuaGamepad>("Gamepad", sol::no_constructor,
			"get_button", &LuaGamepad::getButton,
			"connected", sol::property(&LuaGamepad::isConnected),
			"left_stick", sol::property(&LuaGamepad::getLeftStick),
			"right_stick", sol::property(&LuaGamepad::getRightStick),
			"add_listener", &LuaGamepad::addListener,
			"remove_listener", &LuaGamepad::removeListener
		);
	}
}