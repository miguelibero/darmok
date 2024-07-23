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

	void LuaKeyboard::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down)
	{
		static const std::string desc("on keyboard key");
		for (auto& listener : _listeners[ListenerType::Key])
		{
			checkLuaResult(desc, listener.call(key, modifiers, down));
		}
	}

	void LuaKeyboard::onKeyboardChar(const Utf8Char& chr)
	{
		auto str = chr.toString();
		static const std::string desc("on keyboard char");
		for (auto& listener : _listeners[ListenerType::Char])
		{
			checkLuaResult(desc, listener.call(str));
		}
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

	void LuaKeyboard::registerListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		_listeners[type].push_back(func);
	}

	template<typename T>
	static bool unregisterLuaInputListener(T type, const sol::protected_function& func, std::unordered_map<T, std::vector<sol::protected_function>>& listeners) noexcept
	{
		auto itr = listeners.find(type);
		if (itr == listeners.end())
		{
			return false;
		}
		auto& typeListeners = itr->second;
		auto itr2 = std::remove(typeListeners.begin(), typeListeners.end(), func);
		if (itr2 == typeListeners.end())
		{
			return false;
		}
		typeListeners.erase(itr2, typeListeners.end());
		return true;
	}

	bool LuaKeyboard::unregisterListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		return unregisterLuaInputListener(type, func, _listeners);
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
		if (val.is<KeyboardKey>())
		{
			return val.as<KeyboardKey>();
		}
		if (val.is<std::string>())
		{
			return Keyboard::readKey(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<KeyboardModifier> LuaKeyboard::readModifier(const sol::object& val) noexcept
	{
		if (val.is<KeyboardModifier>())
		{
			return val.as<KeyboardModifier>();
		}
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
		newLuaEnumFunc(lua, "KeyboardKey", KeyboardKey::Count, &Keyboard::getKeyName);
		newLuaEnumFunc(lua, "KeyboardModifier", KeyboardModifier::Count, Keyboard::getModifierName);

		lua.new_usertype<KeyboardInputEvent>("KeyboardInputEvent", sol::default_constructor,
			"key", &KeyboardInputEvent::key,
			"modifiers", &KeyboardInputEvent::modifiers
		);

		lua.new_usertype<LuaKeyboard>("Keyboard", sol::no_constructor,
			"get_key", &LuaKeyboard::getKey,
			"chars", sol::property(&LuaKeyboard::getUpdateChars),
			"register_listener", &LuaKeyboard::registerListener,
			"unregister_listener", &LuaKeyboard::unregisterListener
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

	glm::vec2 LuaMouse::getPositionDelta() const noexcept
	{
		return _mouse.get().getPositionDelta();
	}

	glm::vec2 LuaMouse::getScrollDelta() const noexcept
	{
		return _mouse.get().getScrollDelta();
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

	void LuaMouse::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		static const std::string desc("on mouse position change");
		for (auto& listener : _listeners[ListenerType::Position])
		{
			checkLuaResult(desc, listener.call(delta, absolute));
		}
	}

	void LuaMouse::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		static const std::string desc("on mouse scroll change");
		for (auto& listener : _listeners[ListenerType::Scroll])
		{
			checkLuaResult(desc, listener.call(delta, absolute));
		}
	}

	void LuaMouse::onMouseButton(MouseButton button, bool down)
	{
		static const std::string desc("on mouse button");
		for (auto& listener : _listeners[ListenerType::Button])
		{
			checkLuaResult(desc, listener.call(button, down));
		}
	}

	bool LuaMouse::unregisterListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		return unregisterLuaInputListener(type, func, _listeners);
	}

	void LuaMouse::registerListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		_listeners[type].push_back(func);
	}


	std::optional<MouseInputEvent> LuaMouse::readEvent(const sol::object& val) noexcept
	{
		if (val.is<MouseInputEvent>())
		{
			return val.as<MouseInputEvent>();
		}
		if (val.is<MouseButton>())
		{
			return MouseInputEvent{ val.as<MouseButton>() };
		}
		if (val.is<std::string>())
		{
			if (auto button = Mouse::readButton(val.as<std::string>()))
			{
				return MouseInputEvent{ button.value() };
			}
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
		return std::nullopt;
	}

	std::optional<MouseAnalog> LuaMouse::readAnalog(const sol::object& val) noexcept
	{
		if (val.is<MouseAnalog>())
		{
			return val.as<MouseAnalog>();
		}
		if (val.is<std::string>())
		{
			return Mouse::readAnalog(val.as<std::string>());
		}
		return std::nullopt;
	}

	void LuaMouse::bind(sol::state_view& lua) noexcept
	{
		newLuaEnumFunc(lua, "MouseAnalog", MouseAnalog::Count, &Mouse::getAnalogName);
		newLuaEnumFunc(lua, "MouseButton", MouseButton::Count, &Mouse::getButtonName);

		lua.new_enum<LuaMouseListenerType>("MouseListenerType", {
			{ "Position", LuaMouseListenerType::Position },
			{ "Scroll", LuaMouseListenerType::Scroll },
			{ "Button", LuaMouseListenerType::Button },
		});

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
			"position_delta", sol::property(&LuaMouse::getPositionDelta),
			"scroll", sol::property(&LuaMouse::getScroll),
			"scroll_delta", sol::property(&LuaMouse::getScrollDelta),
			"left_button", sol::property(&LuaMouse::getLeftButton),
			"middle_button", sol::property(&LuaMouse::getMiddleButton),
			"right_button", sol::property(&LuaMouse::getRightButton),
			"get_button", &LuaMouse::getButton,
			"register_listener", &LuaMouse::registerListener,
			"unregister_listener", &LuaMouse::unregisterListener
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

	void LuaGamepad::onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute)
	{
		static const std::string desc("on gamepad stick change");
		for (auto& listener : _listeners[ListenerType::Stick])
		{
			checkLuaResult(desc, listener.call(num, stick, delta, absolute));
		}
	}

	void LuaGamepad::onGamepadButton(uint8_t num, GamepadButton button, bool down)
	{
		static const std::string desc("on gamepad button");
		for (auto& listener : _listeners[ListenerType::Button])
		{
			checkLuaResult(desc, listener.call(num, button, down));
		}
	}

	void LuaGamepad::onGamepadConnect(uint8_t num, bool connected)
	{
		static const std::string desc("on gamepad connect");
		for (auto& listener : _listeners[ListenerType::Connect])
		{
			checkLuaResult(desc, listener.call(num, connected));
		}
	}

	void LuaGamepad::registerListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		_listeners[type].push_back(func);
	}

	bool LuaGamepad::unregisterListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		return unregisterLuaInputListener(type, func, _listeners);
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
		if (val.is<GamepadButton>())
		{
			return val.as<GamepadButton>();
		}
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
		if (val.is<GamepadStick>())
		{
			return val.as<GamepadStick>();
		}
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
		newLuaEnumFunc(lua, "GamepadButton", GamepadButton::Count, &Gamepad::getButtonName);
		newLuaEnumFunc(lua, "GamepadStick", GamepadStick::Count, &Gamepad::getStickName);

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
			"register_listener", &LuaGamepad::registerListener,
			"unregister_listener", &LuaGamepad::unregisterListener
		);
	}
}