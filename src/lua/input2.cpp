#include "lua/input.hpp"

namespace darmok
{
	LuaInputEventListener::LuaInputEventListener(const sol::object& dlg) noexcept
		: _delegate{ dlg, "on_input_event", "input event listener: " }
	{
	}

	void LuaInputEventListener::onInputEvent(const std::string& tag)
	{
		_delegate(tag);
	}

	const LuaDelegate& LuaInputEventListener::getDelegate() const noexcept
	{
		return _delegate;
	}

	LuaInputEventListenerFilter::LuaInputEventListenerFilter(const sol::object& obj, std::optional<std::string> tag) noexcept
		: _obj(obj)
		, _type(entt::type_hash<LuaInputEventListener>::value())
		, _tag(tag)
	{
	}

	bool LuaInputEventListenerFilter::operator()(const std::string& tag, const IInputEventListener& listener) const noexcept
	{
		if (listener.getInputEventListenerType() != _type)
		{
			return false;
		}
		if (_tag && tag != _tag)
		{
			return false;
		}
		return static_cast<const LuaInputEventListener&>(listener).getDelegate() == _obj;
	}

	OptionalRef<Gamepad>::std_t LuaInput::getGamepad(Input& input, uint8_t num) noexcept
	{
		return input.getGamepad(num);
	}

	void LuaInput::addListener(Input& input, const std::string& tag, const sol::object& evObj, const sol::object& listenerObj)
	{
		auto evs = readEvents(evObj);
		if (evs.empty())
		{
			throw sol::error{ "could not parse events" };
		}
		auto listener = std::make_unique<LuaInputEventListener>(listenerObj);
		if (listener->getDelegate())
		{
			input.addListener(tag, evs, std::move(listener));
		}
	}

	bool LuaInput::removeListener1(Input& input, const std::string& tag, const sol::object& listener) noexcept
	{
		return input.removeListeners(LuaInputEventListenerFilter(listener, tag)) > 0;
	}

	bool LuaInput::removeListener2(Input& input, const sol::object& listener) noexcept
	{
		return input.removeListeners(LuaInputEventListenerFilter(listener)) > 0;
	}

	bool LuaInput::checkEvent(const Input& input, const sol::object& val) noexcept
	{
		auto ev = readEvent(val);
		if (!ev)
		{
			return false;
		}
		return input.checkEvent(ev.value());
	}

	bool LuaInput::checkEvents(const Input& input, const sol::table& evs) noexcept
	{
		auto size = evs.size();
		for(auto i = 1; i <= size; ++i)
		{
			if(checkEvent(input, evs[i]))
			{
				return true;
			}
		}
		return false;
	}

	float LuaInput::getAxis(const Input& input, const sol::object& negObj, const sol::object& posObj) noexcept
	{
		auto neg = readDirs(negObj);
		auto pos = readDirs(posObj);
		return input.getAxis(neg, pos);
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
		if (auto ev = LuaGamepad::readButtonEvent(val))
		{
			return ev;
		}
		if (auto ev = LuaGamepad::readStickEvent(val))
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

		LuaUtils::newEnum<InputDirType>(lua, {}, true);

		lua.new_usertype<Input>("Input", sol::no_constructor,
			"keyboard", sol::property(sol::resolve<Keyboard&()>(&Input::getKeyboard)),
			"mouse", sol::property(sol::resolve<Mouse&()>(&Input::getMouse)),
			"gamepads",	sol::property(sol::resolve<Gamepads&()>(&Input::getGamepads)),
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