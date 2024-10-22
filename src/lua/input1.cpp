#include "input.hpp"
#include "window.hpp"
#include "utils.hpp"
#include <darmok/input.hpp>
#include <darmok/string.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>

namespace darmok
{
	LuaKeyboardListener::LuaKeyboardListener(const sol::table& table) noexcept
		: _table(table)
	{
	}

	sol::object LuaKeyboardListener::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaKeyboardListener::_keyDelegate("on_keyboard_key", "running keyboard key listener");
	const LuaTableDelegateDefinition LuaKeyboardListener::_charDelegate("on_keyboard_char", "running keyboard on_keyboard_char listener");

	void LuaKeyboardListener::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down)
	{
		_keyDelegate(_table, modifiers, down);
	}

	void LuaKeyboardListener::onKeyboardChar(const Utf8Char& chr)
	{
		_charDelegate(_table, chr.toString());
	}

	LuaKeyboardListenerFilter::LuaKeyboardListenerFilter(const sol::table& table) noexcept
		: _table(table)
		, _type(entt::type_hash<LuaKeyboardListener>::value())
	{
	}

	bool LuaKeyboardListenerFilter::operator()(const IKeyboardListener& listener) const noexcept
	{
		return listener.getKeyboardListenerType() == _type && static_cast<const LuaKeyboardListener&>(listener).getReal() == _table;
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

	std::string LuaKeyboard::getUpdateChars(const Keyboard& kb) noexcept
	{
		std::stringstream ss;
		for (auto& chr : kb.getUpdateChars())
		{
			ss << chr;
		}
		return ss.str();
	}

	void LuaKeyboard::addListener(Keyboard& kb, const sol::table& table) noexcept
	{
		kb.addListener(std::make_unique<LuaKeyboardListener>(table));
	}

	bool LuaKeyboard::removeListener(Keyboard& kb, const sol::table& table) noexcept
	{
		return kb.removeListeners(LuaKeyboardListenerFilter(table)) > 0;
	}

	void LuaKeyboard::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnumFunc(lua, "KeyboardKey", KeyboardKey::Count, &Keyboard::getKeyName, true);
		LuaUtils::newEnumFunc(lua, "KeyboardModifier", KeyboardModifier::Count, Keyboard::getModifierName, true);

		lua.new_usertype<KeyboardInputEvent>("KeyboardInputEvent", sol::default_constructor,
			"key", &KeyboardInputEvent::key,
			"modifiers", &KeyboardInputEvent::modifiers
		);

		lua.new_usertype<Keyboard>("Keyboard", sol::no_constructor,
			"get_key", &Keyboard::getKey,
			"chars", sol::property(&LuaKeyboard::getUpdateChars),
			"add_listener", &LuaKeyboard::addListener,
			"remove_listener", &LuaKeyboard::removeListener
		);
	}

	LuaMouseListener::LuaMouseListener(const sol::table& table) noexcept
		: _table(table)
	{
	}

	sol::object LuaMouseListener::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaMouseListener::_posDelegate("on_mouse_position_change", "running mouse position change listener");
	const LuaTableDelegateDefinition LuaMouseListener::_scrollDelegate("on_mouse_scroll_change", "running mouse scroll change listener");
	const LuaTableDelegateDefinition LuaMouseListener::_buttonDelegate("on_mouse_button", "running mouse button listener");

	void LuaMouseListener::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		_posDelegate(_table, delta, absolute);
	}

	void LuaMouseListener::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		_scrollDelegate(_table, delta, absolute);
	}

	void LuaMouseListener::onMouseButton(MouseButton button, bool down)
	{
		_buttonDelegate(_table, button, down);
	}

	LuaMouseListenerFilter::LuaMouseListenerFilter(const sol::table& table) noexcept
		: _table(table)
		, _type(entt::type_hash<LuaMouseListener>::value())
	{
	}

	bool LuaMouseListenerFilter::operator()(const IMouseListener& listener) const noexcept
	{
		return listener.getMouseListenerType() == _type && static_cast<const LuaMouseListener&>(listener).getReal() == _table;
	}

	bool LuaMouse::getLeftButton(const Mouse& mouse) noexcept
	{
		return mouse.getButton(MouseButton::Left);
	}

	bool LuaMouse::getMiddleButton(const Mouse& mouse) noexcept
	{
		return mouse.getButton(MouseButton::Middle);
	}

	bool LuaMouse::getRightButton(const Mouse& mouse) noexcept
	{
		return mouse.getButton(MouseButton::Right);
	}

	bool LuaMouse::removeListener(Mouse& mouse, const sol::table& table) noexcept
	{
		return mouse.removeListeners(LuaMouseListenerFilter(table)) > 0;
	}

	void LuaMouse::addListener(Mouse& mouse, const sol::table& table) noexcept
	{
		mouse.addListener(std::make_unique<LuaMouseListener>(table));
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
		
		lua.new_usertype<Mouse>("Mouse", sol::no_constructor,
			"active", sol::property(&Mouse::getActive),
			"position", sol::property(&Mouse::getPosition),
			"velocity", sol::property(&Mouse::getVelocity),
			"scroll", sol::property(&Mouse::getScroll),
			"left_button", sol::property(&LuaMouse::getLeftButton),
			"middle_button", sol::property(&LuaMouse::getMiddleButton),
			"right_button", sol::property(&LuaMouse::getRightButton),
			"get_button", &Mouse::getButton,
			"add_listener", &LuaMouse::addListener,
			"remove_listener", &LuaMouse::removeListener
		);
	}

	LuaGamepadListener::LuaGamepadListener(const sol::table& table) noexcept
		: _table(table)
	{
	}

	sol::object LuaGamepadListener::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaGamepadListener::_stickDelegate("on_gamepad_stick_change", "running gamepad stick change listener");
	const LuaTableDelegateDefinition LuaGamepadListener::_buttonDelegate("on_gamepad_button", "running gamepad button listener");
	const LuaTableDelegateDefinition LuaGamepadListener::_connectDelegate("on_gamepad_connect", "running gamepad connect listener");

	void LuaGamepadListener::onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute)
	{
		_stickDelegate(_table, num, stick, delta, absolute);
	}

	void LuaGamepadListener::onGamepadButton(uint8_t num, GamepadButton button, bool down)
	{
		_buttonDelegate(_table, num, button, down);
	}

	void LuaGamepadListener::onGamepadConnect(uint8_t num, bool connected)
	{
		_connectDelegate(_table, num, connected);
	}

	LuaGamepadListenerFilter::LuaGamepadListenerFilter(const sol::table& table) noexcept
		: _table(table)
		, _type(entt::type_hash<LuaGamepadListener>::value())
	{
	}

	bool LuaGamepadListenerFilter::operator()(const IGamepadListener& listener) const noexcept
	{
		return listener.getGamepadListenerType() == _type && static_cast<const LuaGamepadListener&>(listener).getReal() == _table;
	}

	void LuaGamepad::addListener(Gamepad& gamepad, const sol::table& table) noexcept
	{
		gamepad.addListener(std::make_unique<LuaGamepadListener>(table));
	}

	bool LuaGamepad::removeListener(Gamepad& gamepad, const sol::table& table) noexcept
	{
		return gamepad.removeListeners(LuaGamepadListenerFilter(table)) > 0;
	}

	const glm::vec3& LuaGamepad::getLeftStick(const Gamepad& gamepad) noexcept
	{
		return gamepad.getStick(GamepadStick::Left);
	}

	const glm::vec3& LuaGamepad::getRightStick(const Gamepad& gamepad) noexcept
	{
		return gamepad.getStick(GamepadStick::Right);
	}

	std::optional<GamepadButton> LuaGamepad::readButton(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Gamepad::readButton(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<GamepadInputEvent> LuaGamepad::readButtonEvent(const sol::object& val) noexcept
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

	std::optional<GamepadStickInputEvent> LuaGamepad::readStickEvent(const sol::object& val) noexcept
	{
		if (val.is<GamepadStickInputEvent>())
		{
			return val.as<GamepadStickInputEvent>();
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
		auto gamepad = Gamepad::Any;
		if (size > 1 && tab[3].is<int>())
		{
			gamepad = tab[3];
		}
		return GamepadStickInputEvent{ stick.value(), dirType.value(), gamepad };
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

		lua.new_usertype<Gamepad>("Gamepad", sol::no_constructor,
			"get_button", &Gamepad::getButton,
			"connected", sol::property(&Gamepad::isConnected),
			"left_stick", sol::property(&LuaGamepad::getLeftStick),
			"right_stick", sol::property(&LuaGamepad::getRightStick),
			"add_listener", &LuaGamepad::addListener,
			"remove_listener", &LuaGamepad::removeListener
		);
	}
}