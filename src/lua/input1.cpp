#include "lua/input.hpp"
#include "lua/window.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include <darmok/input.hpp>
#include <darmok/string.hpp>

#include <sstream>

#include <glm/gtx/string_cast.hpp>

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

	const LuaTableDelegateDefinition LuaKeyboardListener::_keyDelegate{ "on_keyboard_key", "running keyboard key listener" };
	const LuaTableDelegateDefinition LuaKeyboardListener::_charDelegate{ "on_keyboard_char", "running keyboard on_keyboard_char listener" };

	expected<void, std::string> LuaKeyboardListener::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept
	{
		return _keyDelegate.tryGet<void>(_table, modifiers, down);
	}

	expected<void, std::string> LuaKeyboardListener::onKeyboardChar(char32_t chr) noexcept
	{
		return _charDelegate.tryGet<void>(_table, StringUtils::toUtf8(chr));
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
			KeyboardInputEvent ev;
			ev.set_key(key.value());
			return ev;
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
		KeyboardInputEvent ev;
		ev.set_key(key.value());
		for (size_t i = 2; i <= size; i++)
		{
			if (auto mod = readModifier(tab[i]))
			{
				ev.mutable_modifiers()->Add(mod.value());
			}
		}
		return ev;
	}

	std::string LuaKeyboard::getUpdateChars(const Keyboard& kb) noexcept
	{
		return StringUtils::toUtf8(kb.getUpdateChars());
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
		LuaUtils::newEnum<KeyboardKey>(lua, {}, true);
		LuaUtils::newEnum<KeyboardModifier>(lua, {}, true);

		LuaProtobufBinding<KeyboardInputEvent>{ lua, "KeyboardInputEvent" };

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

	const LuaTableDelegateDefinition LuaMouseListener::_posDelegate{ "on_mouse_position_change", "running mouse position change listener" };
	const LuaTableDelegateDefinition LuaMouseListener::_scrollDelegate{ "on_mouse_scroll_change", "running mouse scroll change listener" };
	const LuaTableDelegateDefinition LuaMouseListener::_buttonDelegate{ "on_mouse_button", "running mouse button listener" };

	expected<void, std::string> LuaMouseListener::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
	{
		return _posDelegate.tryGet<void>(_table, delta, absolute);
	}

	expected<void, std::string> LuaMouseListener::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
	{
		return _scrollDelegate.tryGet<void>(_table, delta, absolute);
	}

	expected<void, std::string> LuaMouseListener::onMouseButton(MouseButton button, bool down) noexcept
	{
		return _buttonDelegate.tryGet<void>(_table, button, down);
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
		return mouse.getButton(Mouse::Definition::LeftButton);
	}

	bool LuaMouse::getMiddleButton(const Mouse& mouse) noexcept
	{
		return mouse.getButton(Mouse::Definition::MiddleButton);
	}

	bool LuaMouse::getRightButton(const Mouse& mouse) noexcept
	{
		return mouse.getButton(Mouse::Definition::RightButton);
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
			MouseInputEvent ev;
			ev.set_button(button.value());
			return ev;
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
		MouseInputDir dir;
		dir.set_analog(analog.value());
		dir.set_type(dirType.value());
		return dir;
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
		LuaUtils::newEnum<MouseAnalog>(lua, {}, true);
		LuaUtils::newEnum<MouseButton>(lua, {}, true);

		LuaProtobufBinding<MouseInputEvent>{ lua, "MouseInputEvent" };
		LuaProtobufBinding<MouseInputDir>{ lua, "MouseInputDir" };

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

	expected<void, std::string> LuaGamepadListener::onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) noexcept
	{
		return _stickDelegate.tryGet<void>(_table, num, stick, delta, absolute);
	}

	expected<void, std::string> LuaGamepadListener::onGamepadButton(uint8_t num, GamepadButton button, bool down) noexcept
	{
		return _buttonDelegate.tryGet<void>(_table, num, button, down);
	}

	expected<void, std::string> LuaGamepadListener::onGamepadConnect(uint8_t num, bool connected) noexcept
	{
		return _connectDelegate.tryGet<void>(_table, num, connected);
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
		return gamepad.getStick(GamepadStick::Gamepad_Stick_LeftStick);
	}

	const glm::vec3& LuaGamepad::getRightStick(const Gamepad& gamepad) noexcept
	{
		return gamepad.getStick(GamepadStick::Gamepad_Stick_RightStick);
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
			GamepadInputEvent ev;
			ev.set_button(button.value());
			return ev;
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
		GamepadInputEvent ev;
		ev.set_button(button.value());
		ev.set_gamepad(gamepad);
		return ev;
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
		GamepadStickInputEvent ev;
		ev.set_stick(stick.value());
		ev.set_dir(dirType.value());
		ev.set_gamepad(gamepad);
		return ev;
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
		GamepadInputDir dir;
		dir.set_stick(stick.value());
		dir.set_type(dirType.value());
		dir.set_gamepad(gamepad);
		return dir;
	}

	void LuaGamepad::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnum<GamepadButton>(lua, {}, true);
		LuaUtils::newEnum<GamepadStick>(lua, {}, true);

		LuaProtobufBinding<GamepadInputEvent>{ lua, "GamepadInputEvent" };
		LuaProtobufBinding<GamepadStickInputEvent>{ lua, "GamepadStickInputEvent" };
		LuaProtobufBinding<GamepadInputDir>{ lua, "GamepadInputDir" };

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