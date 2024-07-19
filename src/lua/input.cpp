#include "input.hpp"
#include "window.hpp"
#include "utils.hpp"
#include <darmok/input.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>

namespace darmok
{
	LuaKeyboard::LuaKeyboard(Keyboard& kb) noexcept
		: _kb(kb)
	{
	}

	bool LuaKeyboard::getKey(const sol::variadic_args& args) const noexcept
	{
		for (auto& arg : args)
		{
			KeyboardBindingKey binding;
			if (arg.is<std::string>())
			{
				auto opt = KeyboardBindingKey::read(arg.as<std::string>());
				if (!opt)
				{
					return false;
				}
				binding = opt.value();
			}
			else if (arg.is<KeyboardKey>())
			{
				binding.key = arg.as<KeyboardKey>();
			}
			else
			{
				return false;
			}
			if (_kb.get().getKey(binding.key, binding.modifiers))
			{
				return true;
			}
		}
		return false;
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

	std::vector<std::pair<std::string_view, KeyboardKey>> LuaKeyboard::getKeyboardKeys() noexcept
	{
		std::vector<std::pair<std::string_view, KeyboardKey>> keys;
		for (int i = 0; i < to_underlying(KeyboardKey::Count); i++)
		{
			auto key = (KeyboardKey)i;
			keys.emplace_back(Keyboard::getKeyName(key), key);
		}
		return keys;
	}

	void LuaKeyboard::bind(sol::state_view& lua) noexcept
	{
		auto keys = getKeyboardKeys();
		lua.new_enum<KeyboardKey>("KeyboardKey",
			std::initializer_list<std::pair<std::string_view, KeyboardKey>>(&keys.front(), &keys.front() + keys.size()));
		lua.new_usertype<LuaKeyboard>("Keyboard", sol::no_constructor,
			"get_key", &LuaKeyboard::getKey,
			"chars", sol::property(&LuaKeyboard::getUpdateChars)
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
		auto itr = _listeners.find(type);
		if (itr == _listeners.end())
		{
			return false;
		}
		auto& listeners = itr->second;
		auto itr2 = std::remove(listeners.begin(), listeners.end(), func);
		if (itr2 == listeners.end())
		{
			return false;
		}
		listeners.erase(itr2, listeners.end());
		return true;
	}

	void LuaMouse::registerListener(ListenerType type, const sol::protected_function& func) noexcept
	{
		_listeners[type].push_back(func);
	}

	void LuaMouse::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<MouseButton>("MouseButton", {
			{ "Left", MouseButton::Left },
			{ "Middle", MouseButton::Middle },
			{ "Right", MouseButton::Right },
		});

		lua.new_enum<LuaMouseListenerType>("MouseListenerType", {
			{ "Position", LuaMouseListenerType::Position },
			{ "Scroll", LuaMouseListenerType::Scroll },
			{ "Button", LuaMouseListenerType::Button },
		});

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
	}

	bool LuaGamepad::getButton(const std::string& name) const noexcept
	{
		auto button = GamepadBindingKey::readButton(name);
		if (!button)
		{
			return false;
		}
		return _gamepad.get().getButton(button.value());
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

	void LuaGamepad::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaGamepad>("Gamepad", sol::no_constructor,
			"get_button", &LuaGamepad::getButton,
			"connected", sol::property(&LuaGamepad::isConnected),
			"left_stick", sol::property(&LuaGamepad::getLeftStick),
			"right_stick", sol::property(&LuaGamepad::getRightStick)
		);
	}

    LuaInput::LuaInput(Input& input) noexcept
		: _input(input)
		, _keyboard(input.getKeyboard())
		, _mouse(input.getMouse())
	{
		auto& gamepads = input.getGamepads();
		_gamepads.reserve(gamepads.size());
		for (auto& gamepad : gamepads)
		{
			_gamepads.emplace_back(gamepad);
		}
	}

	static void callLuaBinding(std::string key, sol::protected_function fn)
	{
		auto result = fn();
		if (!result.valid())
		{
			logLuaError(std::string("triggering input binding '") + key + "'", result);
		}
	}

	void LuaInput::addBindings(const std::string& name, const sol::table& data) noexcept
	{
		std::vector<InputBinding> bindings;

		for (auto& elm : data)
		{
			if (elm.second.get_type() != sol::type::function)
			{
				continue;
			}
			auto key = elm.first.as<std::string>();
			auto func = elm.second.as<sol::protected_function>();
			auto binding = InputBinding::read(key, std::bind(&callLuaBinding, key, func));
			if (binding)
			{
				bindings.push_back(std::move(binding.value()));
			}
		}
		_input->addBindings(name, std::move(bindings));
	}

	LuaKeyboard& LuaInput::getKeyboard() noexcept
	{
		return _keyboard;
	}

	LuaMouse& LuaInput::getMouse() noexcept
	{
		return _mouse;
	}

	OptionalRef<LuaGamepad>::std_t LuaInput::getGamepad(uint8_t num) noexcept
	{
		if (num >= 0 && num < _gamepads.size())
		{
			return _gamepads[num];
		}
		return std::nullopt;
	}

	const std::vector<LuaGamepad>& LuaInput::getGamepads() noexcept
	{
		return _gamepads;
	}

	void LuaInput::bind(sol::state_view& lua) noexcept
	{
		LuaKeyboard::bind(lua);
		LuaMouse::bind(lua);
		LuaGamepad::bind(lua);

		lua.new_usertype<LuaInput>("Input", sol::no_constructor,
			"add_bindings",		&LuaInput::addBindings,
			"keyboard",			sol::property(&LuaInput::getKeyboard),
			"mouse",			sol::property(&LuaInput::getMouse),
			"gamepads",			sol::property(&LuaInput::getGamepads),
			"get_gamepad",		&LuaInput::getGamepad
		);
	}
}