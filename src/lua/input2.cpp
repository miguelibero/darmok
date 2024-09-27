#include "input.hpp"

namespace darmok
{
	LuaInputEventListener::LuaInputEventListener(const std::string& tag, const sol::object& dlg) noexcept
		: _delegate(dlg, "on_input_event")
		, _tag(tag)
	{
	}

	void LuaInputEventListener::onInputEvent(const std::string& tag)
	{
		auto desc = "input event listener: " + tag;
		LuaUtils::checkResult(desc, _delegate(tag));
	}

	const std::string& LuaInputEventListener::getTag() const noexcept
	{
		return _tag;
	}

	const LuaDelegate& LuaInputEventListener::getDelegate() const noexcept
	{
		return _delegate;
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
			_input->removeListener(*listener);
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

	void LuaInput::addListener(const std::string& tag, const sol::object& evObj, const sol::object& listenerObj)
	{
		auto evs = readEvents(evObj);
		if (evs.empty())
		{
			throw std::invalid_argument("could not parse events");
		}
		auto listener = std::make_unique<LuaInputEventListener>(tag, listenerObj);
		if (listener->getDelegate())
		{
			_input->addListener(tag, evs, *listener);
			_listeners.push_back(std::move(listener));
		}
	}

	bool LuaInput::removeListener1(const std::string& tag, const sol::object& listener) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&tag, &listener](auto& elm) {
			return elm->getTag() == tag && elm->getDelegate() == listener;
		});
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr, _listeners.end());
		return true;
	}

	bool LuaInput::removeListener2(const sol::object& listener) noexcept
	{
		auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [&listener](auto& elm) {
			return elm->getDelegate() == listener;
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

	bool LuaInput::checkEvents(const sol::table& evs) const noexcept
	{
		auto size = evs.size();
		for(auto i = 1; i <= size; ++i)
		{
			if(checkEvent(evs[i]))
			{
				return true;
			}
		}
		return false;
	}

	float LuaInput::getAxis(const sol::object& posObj, const sol::object& negObj) const noexcept
	{
		auto pos = readDirs(posObj);
		auto neg = readDirs(negObj);
		return _input->getAxis(pos, neg);
	}

	InputEvents LuaInput::readEvents(const sol::object& val) noexcept
	{
		InputEvents evs;
		if (val.get_type() == sol::type::table)
		{
			auto tab = val.as<sol::table>();
			auto size = tab.size();
			for (auto i = 1; i <= size; ++i)
			{
				auto ev = readEvent(tab[i]);
				if (ev)
				{
					evs.push_back(ev.value());
				}
			}
		}
		else
		{
			auto ev = readEvent(val);
			if (ev)
			{
				evs.push_back(ev.value());
			}
		}
		return evs;
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
		if (val.get_type() == sol::type::table)
		{
			auto tab = val.as<sol::table>();
			auto size = tab.size();
			for(auto i = 1; i <= size; ++i)
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

		LuaUtils::newEnumFunc(lua, "InputDirType", InputDirType::Count, &Input::getDirTypeName, true);

		lua.new_usertype<LuaInput>("Input", sol::no_constructor,
			"keyboard", sol::property(&LuaInput::getKeyboard),
			"mouse", sol::property(&LuaInput::getMouse),
			"gamepads",	sol::property(&LuaInput::getGamepads),
			"get_gamepad", &LuaInput::getGamepad,
			"get_axis", &LuaInput::getAxis,
			"check_event", &LuaInput::checkEvent,
			"check_events", &LuaInput::checkEvents,
			"add_listener", &LuaInput::addListener,
			"remove_listener", sol::overload(
				&LuaInput::removeListener1,
				&LuaInput::removeListener2
			)
		);
	}
}