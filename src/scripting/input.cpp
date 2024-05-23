#include "input.hpp"
#include "window.hpp"
#include "utils.hpp"
#include <darmok/input.hpp>
#include <glm/gtx/string_cast.hpp>

namespace darmok
{
	LuaKeyboard::LuaKeyboard(Keyboard& kb) noexcept
		: _kb(kb)
	{
		glm::to_string(glm::vec3(0));
	}

	bool LuaKeyboard::getKey(const std::string& name) const noexcept
	{
		auto optBinding = KeyboardBindingKey::read(name);
		if (!optBinding)
		{
			return false;
		}
		auto binding = optBinding.value();
		return _kb->getKey(binding.key, binding.modifiers);
	}

	std::string LuaKeyboard::getUpdateChars() const noexcept
	{
		std::string str;
		for (auto& chr : _kb->getUpdateChars())
		{
			str += chr.stringView();
		}
		return str;
	}

	void LuaKeyboard::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaKeyboard>("Keyboard",
			sol::constructors<>(),
			"get_key", &LuaKeyboard::getKey,
			"chars", sol::property(&LuaKeyboard::getUpdateChars)
		);
	}

	LuaMouse::LuaMouse(Mouse& mouse) noexcept
		: _mouse(mouse)
	{
		_mouse->addListener(*this);
	}

	LuaMouse::~LuaMouse() noexcept
	{
		if (_mouse)
		{
			_mouse->removeListener(*this);
		}
	}

	bool LuaMouse::getActive() const noexcept
	{
		return _mouse->getActive();
	}

	const glm::vec2& LuaMouse::getPosition() const noexcept
	{
		return _mouse->getPosition();
	}

	glm::vec2 LuaMouse::getPositionDelta() const noexcept
	{
		return _mouse->getPositionDelta();
	}

	glm::vec2 LuaMouse::getScrollDelta() const noexcept
	{
		return _mouse->getScrollDelta();
	}

	const glm::vec2& LuaMouse::getScroll() const noexcept
	{
		return _mouse->getScroll();
	}

	bool LuaMouse::getLeftButton() const noexcept
	{
		return _mouse->getButton(MouseButton::Left);
	}

	bool LuaMouse::getMiddleButton() const noexcept
	{
		return _mouse->getButton(MouseButton::Middle);
	}

	bool LuaMouse::getRightButton() const noexcept
	{
		return _mouse->getButton(MouseButton::Right);
	}

	void LuaMouse::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		for (auto& listener : _positionListeners)
		{
			listener.call(delta, absolute);
		}
	}

	void LuaMouse::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute)
	{
		for (auto& listener : _buttonListeners)
		{
			listener.call(delta, absolute);
		}
	}

	void LuaMouse::onMouseButton(MouseButton button, bool down)
	{
		for (auto& listener : _buttonListeners)
		{
			listener.call(button, down);
		}
	}

	void LuaMouse::registerPositionListener(const sol::protected_function& func) noexcept
	{
		_positionListeners.push_back(func);
	}

	template<typename T>
	bool unregisterListener(std::vector<T> listeners, const T& elm) noexcept
	{
		auto itr = std::remove(listeners.begin(), listeners.end(), elm);
		if (itr == listeners.end())
		{
			return false;
		}
		listeners.erase(itr, listeners.end());
		return true;
	}

	bool LuaMouse::unregisterPositionListener(const sol::protected_function& func) noexcept
	{
		return unregisterListener(_positionListeners, func);
	}

	void LuaMouse::registerScrollListener(const sol::protected_function& func) noexcept
	{
		_scrollListeners.push_back(func);
	}

	bool LuaMouse::unregisterScrollListener(const sol::protected_function& func) noexcept
	{
		return unregisterListener(_scrollListeners, func);
	}

	void LuaMouse::registerButtonListener(const sol::protected_function& func) noexcept
	{
		_buttonListeners.push_back(func);
	}

	bool LuaMouse::unregisterButtonListener(const sol::protected_function& func) noexcept
	{
		return unregisterListener(_buttonListeners, func);
	}

	void LuaMouse::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<MouseButton>("MouseButton", {
			{ "left", MouseButton::Left },
			{ "middle", MouseButton::Middle },
			{ "right", MouseButton::Right },
		});

		lua.new_usertype<LuaMouse>("Mouse",
			sol::constructors<>(),
			"active", sol::property(&LuaMouse::getActive),
			"position", sol::property(&LuaMouse::getPosition),
			"position_delta", sol::property(&LuaMouse::getPositionDelta),
			"scroll", sol::property(&LuaMouse::getScroll),
			"scroll_delta", sol::property(&LuaMouse::getScrollDelta),
			"left_button", sol::property(&LuaMouse::getLeftButton),
			"middle_button", sol::property(&LuaMouse::getMiddleButton),
			"right_button", sol::property(&LuaMouse::getRightButton),
			"register_position_listener", &LuaMouse::registerPositionListener,
			"unregister_position_listener", &LuaMouse::unregisterPositionListener,
			"register_scroll_listener", &LuaMouse::registerScrollListener,
			"unregister_scroll_listener", &LuaMouse::unregisterScrollListener,
			"register_button_listener", &LuaMouse::registerButtonListener,
			"unregister_button_listener", &LuaMouse::unregisterButtonListener
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
		return _gamepad->getButton(button.value());
	}

	const glm::ivec3& LuaGamepad::getLeftStick() const noexcept
	{
		return _gamepad->getStick(GamepadStick::Left);
	}

	const glm::ivec3& LuaGamepad::getRightStick() const noexcept
	{
		return _gamepad->getStick(GamepadStick::Right);
	}

	bool LuaGamepad::isConnected() const noexcept
	{
		return _gamepad->isConnected();
	}

	void LuaGamepad::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaGamepad>("Gamepad",
			sol::constructors<>(),
			"get_button", &LuaGamepad::getButton,
			"connected", sol::property(&LuaGamepad::isConnected),
			"left_stick", sol::property(&LuaGamepad::getLeftStick),
			"right_stick", sol::property(&LuaGamepad::getRightStick)
		);
	}

    LuaInput::LuaInput(Input& input) noexcept
		: _input(input)
	{
	}

	static void callLuaBinding(std::string key, sol::protected_function fn)
	{
		auto result = fn();
		if (!result.valid())
		{
			recoveredLuaError(std::string("triggering input binding '") + key + "'", result);
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

	LuaKeyboard LuaInput::getKeyboard() noexcept
	{
		return LuaKeyboard(_input->getKeyboard());
	}

	LuaMouse LuaInput::getMouse() noexcept
	{
		return LuaMouse(_input->getMouse());
	}

	std::optional<LuaGamepad> LuaInput::getGamepad(uint8_t num) noexcept
	{
        auto gamepad = _input->getGamepad(num);
        if(gamepad)
        {
            return LuaGamepad(gamepad.value());
        }
		return std::nullopt;
	}

	std::vector<LuaGamepad> LuaInput::getGamepads() noexcept
	{
        std::vector<LuaGamepad> gamepads;
        for(auto& gamepad : _input->getGamepads())
        {
            gamepads.push_back(LuaGamepad(gamepad));
        }
		return gamepads;
	}

	void LuaInput::bind(sol::state_view& lua) noexcept
	{

		LuaKeyboard::bind(lua);
		LuaMouse::bind(lua);
		LuaGamepad::bind(lua);

		lua.new_usertype<LuaInput>("Input",
			sol::constructors<>(),
			"add_bindings",		&LuaInput::addBindings,
			"keyboard",			sol::property(&LuaInput::getKeyboard),
			"mouse",			sol::property(&LuaInput::getMouse),
			"gamepads",			sol::property(&LuaInput::getGamepads),
			"get_gamepad",		&LuaInput::getGamepad
		);
	}
}