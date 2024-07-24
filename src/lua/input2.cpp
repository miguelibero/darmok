#include "input.hpp"
#include "utils.hpp"

namespace darmok
{
	LuaInputEventListener::LuaInputEventListener(const std::string& tag, const sol::protected_function& func) noexcept
		: _func(func)
		, _tag(tag)
	{
	}

	void LuaInputEventListener::onInputEvent(const std::string& tag)
	{
		auto desc = "on input event " + tag;
		checkLuaResult(desc, _func.call(tag));
	}

	const std::string& LuaInputEventListener::getTag() const noexcept
	{
		return _tag;
	}

	const sol::protected_function& LuaInputEventListener::getFunc() const noexcept
	{
		return _func;
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

	LuaInput::~LuaInput() noexcept
	{
		for (auto& listener : _listeners)
		{
			_input->removeListener(listener);
		}
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

	void LuaInput::registerListener(const std::string& tag, const sol::object& evObj, const sol::protected_function& func)
	{
		auto ev = readEvent(evObj);
		if (!ev)
		{
			throw std::invalid_argument("could not parse event");
		}
		auto& listener = _listeners.emplace_back(tag, func);
		_input->addListener(tag, ev.value(), listener);
	}

	bool LuaInput::unregisterListener1(const std::string& tag, const sol::protected_function& func) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&tag, &func](auto& listener) {
			return listener.getTag() == tag && listener.getFunc() == func;
		});
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool LuaInput::unregisterListener2(const sol::protected_function& func) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&func](auto& listener) {
			return listener.getFunc() == func;
		});
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool LuaInput::checkEvent(const sol::object& val) const noexcept
	{
		auto ev = readEvent(val);
		if (!ev)
		{
			return false;
		}
		return _input->checkEvent(ev.value());
	}

	float LuaInput::getAxis(const sol::object& posObj, const sol::object& negObj) const noexcept
	{
		auto pos = readDirs(posObj);
		auto neg = readDirs(negObj);
		return _input->getAxis(pos, neg);
	}

	std::optional<InputEvent> LuaInput::readEvent(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			if (auto ev = Input::readEvent(val.as<std::string>()))
			{
				return ev.value();
			}
		}
		if (auto ev = LuaKeyboard::readEvent(val))
		{
			return ev;
		}
		if (auto ev = LuaMouse::readEvent(val))
		{
			return ev;
		}
		if (auto ev = LuaGamepad::readEvent(val))
		{
			return ev;
		}
		return std::nullopt;
	}

	std::optional<InputDirType> LuaInput::readDirType(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			return Input::readDirType(val.as<std::string>());
		}
		return std::nullopt;
	}

	std::optional<InputDir> LuaInput::readDir(const sol::object& val) noexcept
	{
		if (val.is<std::string>())
		{
			if (auto dir = Input::readDir(val.as<std::string>()))
			{
				return dir.value();
			}
		}
		if (auto dir = LuaMouse::readDir(val))
		{
			return dir;
		}
		if (auto dir = LuaGamepad::readDir(val))
		{
			return dir;
		}
		if (auto ev = readEvent(val))
		{
			return ev;
		}
		return std::nullopt;
	}

	InputDirs LuaInput::readDirs(const sol::object& val) noexcept
	{
		InputDirs dirs;
		if (val.is<sol::table>())
		{
			auto tab = val.as<sol::table>();
			auto size = tab.size();
			for(auto i = 1; i <= size; i++)
			{
				auto dir = readDir(tab[i]);
				if (dir)
				{
					dirs.push_back(dir.value());
				}
			}
		}
		else
		{
			auto dir = readDir(val);
			if (dir)
			{
				dirs.push_back(dir.value());
			}
		}
		return dirs;
	}

	void LuaInput::bind(sol::state_view& lua) noexcept
	{
		LuaKeyboard::bind(lua);
		LuaMouse::bind(lua);
		LuaGamepad::bind(lua);

		newLuaEnumFunc(lua, "InputDirType", InputDirType::Count, &Input::getDirTypeName, true);

		lua.new_usertype<LuaInput>("Input", sol::no_constructor,
			"keyboard", sol::property(&LuaInput::getKeyboard),
			"mouse", sol::property(&LuaInput::getMouse),
			"gamepads",	sol::property(&LuaInput::getGamepads),
			"get_gamepad", &LuaInput::getGamepad,
			"get_axis", &LuaInput::getAxis,
			"check_event", &LuaInput::checkEvent,
			"register_listener", &LuaInput::registerListener,
			"unregister_listener", sol::overload(
				&LuaInput::unregisterListener1,
				&LuaInput::unregisterListener2
			)
		);
	}
}