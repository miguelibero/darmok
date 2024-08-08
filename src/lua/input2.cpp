#include "input.hpp"
#include "utils.hpp"

namespace darmok
{
	LuaFunctionInputEventListener::LuaFunctionInputEventListener(const std::string& tag, const sol::protected_function& func) noexcept
		: _func(func)
		, _tag(tag)
	{
	}

	void LuaFunctionInputEventListener::onInputEvent(const std::string& tag)
	{
		auto desc = "input event function listener: " + tag;
		LuaUtils::checkResult(desc, _func.call(tag));
	}

	const std::string& LuaFunctionInputEventListener::getTag() const noexcept
	{
		return _tag;
	}

	const sol::protected_function& LuaFunctionInputEventListener::getFunction() const noexcept
	{
		return _func;
	}

	LuaTableInputEventListener::LuaTableInputEventListener(const std::string& tag, const sol::table& table) noexcept
		: _tag(tag)
		, _table(table)
	{
	}

	void LuaTableInputEventListener::onInputEvent(const std::string& tag)
	{
		auto desc = "input event table listener: " + tag;
		LuaUtils::callTableDelegate(_table, "on_input_event", desc, [&tag](auto& func, auto& self) {
			return func(self, tag);
		});
	}

	const std::string& LuaTableInputEventListener::getTag() const noexcept
	{
		return _tag;
	}

	const sol::table& LuaTableInputEventListener::getTable() const noexcept
	{
		return _table;
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
		for (auto& listener : _funcListeners)
		{
			_input->removeListener(*listener);
		}
		for (auto& listener : _tabListeners)
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

	void LuaInput::addListener1(const std::string& tag, const sol::object& evObj, const sol::protected_function& func)
	{
		auto evs = readEvents(evObj);
		if (evs.empty())
		{
			throw std::invalid_argument("could not parse events");
		}
		auto& listener = *_funcListeners.emplace_back(std::make_unique<LuaFunctionInputEventListener>(tag, func));
		_input->addListener(tag, evs, listener);
	}

	void LuaInput::addListener2(const std::string& tag, const sol::object& evObj, const sol::table& tab)
	{
		auto evs = readEvents(evObj);
		if (evs.empty())
		{
			throw std::invalid_argument("could not parse events");
		}
		auto& listener = *_tabListeners.emplace_back(std::make_unique<LuaTableInputEventListener>(tag, tab));
		_input->addListener(tag, evs, listener);
	}

	bool LuaInput::removeListener1(const std::string& tag, const sol::protected_function& func) noexcept
	{
		auto itr = std::remove_if(_funcListeners.begin(), _funcListeners.end(), [&tag, &func](auto& listener) {
			return listener->getTag() == tag && listener->getFunction() == func;
		});
		if (itr == _funcListeners.end())
		{
			return false;
		}
		_funcListeners.erase(itr, _funcListeners.end());
		return true;
	}

	bool LuaInput::removeListener2(const sol::protected_function& func) noexcept
	{
		auto itr = std::remove_if(_funcListeners.begin(), _funcListeners.end(), [&func](auto& listener) {
			return listener->getFunction() == func;
		});
		if (itr == _funcListeners.end())
		{
			return false;
		}
		_funcListeners.erase(itr, _funcListeners.end());
		return true;
	}

	bool LuaInput::removeListener3(const std::string& tag, const sol::table& table) noexcept
	{
		auto itr = std::remove_if(_tabListeners.begin(), _tabListeners.end(), [&tag, &table](auto& listener) {
			return listener->getTag() == tag && listener->getTable() == table;
			});
		if (itr == _tabListeners.end())
		{
			return false;
		}
		_tabListeners.erase(itr, _tabListeners.end());
		return true;
	}

	bool LuaInput::removeListener4(const sol::table& table) noexcept
	{
		auto itr = std::remove_if(_tabListeners.begin(), _tabListeners.end(), [&table](auto& listener) {
			return listener->getTable() == table;
			});
		if (itr == _tabListeners.end())
		{
			return false;
		}
		_tabListeners.erase(itr, _tabListeners.end());
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
			"add_listener", sol::overload(&LuaInput::addListener1, &LuaInput::addListener2),
			"remove_listener", sol::overload(
				&LuaInput::removeListener1,
				&LuaInput::removeListener2,
				&LuaInput::removeListener3,
				&LuaInput::removeListener4
			)
		);
	}
}