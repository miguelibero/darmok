#include "component.hpp"
#include "utils.hpp"
#include "app.hpp"
#include "scene.hpp"

namespace darmok
{
    const std::string LuaComponentContainer::_typeKey = "class";

	LuaComponentContainer::Components::iterator LuaComponentContainer::find(Key key) noexcept
	{
		return std::find_if(_components.begin(), _components.end(), [&key](auto& elm) { return elm.first == key; });
	}

	LuaComponentContainer::Components::const_iterator LuaComponentContainer::find(Key key) const noexcept
	{
		return std::find_if(_components.begin(), _components.end(), [&key](auto& elm) { return elm.first == key; });
	}

	bool LuaComponentContainer::remove(const sol::object& type) noexcept
	{
		auto itr = find(getKey(type));
		if (itr == _components.end())
		{
			return false;
		}
		_components.erase(itr);
		return true;
	}

	bool LuaComponentContainer::contains(const sol::object& type) const noexcept
	{
		auto itr = find(getKey(type));
		return itr != _components.end();
	}

	void LuaComponentContainer::add(const sol::table& comp)
	{
		auto key = getKey(comp[_typeKey]);
		auto itr = find(key);
		if (itr != _components.end())
		{
			throw std::invalid_argument("component of type already exists");
		}
		_components.emplace_back(key, comp);
	}

	sol::object LuaComponentContainer::get(const sol::object& type) noexcept
	{
		auto itr = find(getKey(type));
		if (itr == _components.end())
		{
			return sol::nil;
		}
		return itr->second;
	}

	LuaComponentContainer::Key LuaComponentContainer::getKey(const sol::object& type) noexcept
	{
		return type.pointer();
	}

	const LuaTableDelegateDefinition LuaAppComponentContainer::_initDef("init", "app component init");
	const LuaTableDelegateDefinition LuaAppComponentContainer::_shutdownDef("shutdown", "app component shutdown");
	const LuaTableDelegateDefinition LuaAppComponentContainer::_renderResetDef("render_reset", "app component render reset");
	const LuaTableDelegateDefinition LuaAppComponentContainer::_updateDef("update", "app component update");

	void LuaAppComponentContainer::init(App& app)
	{
		for (auto& [type, comp] : _components)
		{
			_initDef(comp);
		}
	}

	void LuaAppComponentContainer::shutdown()
	{
		for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
		{
			_shutdownDef(itr->second);
		}
	}

	void LuaAppComponentContainer::renderReset()
	{
		for (auto& [type, comp] : _components)
		{
			_renderResetDef(comp);
		}
	}

	void LuaAppComponentContainer::update(float deltaTime)
	{
		for (auto& [type, comp] : _components)
		{
			_updateDef(comp, deltaTime);
		}
	}

	LuaSceneComponentContainer::LuaSceneComponentContainer(LuaScene& scene) noexcept
		: _scene(scene)
	{
	}


	const LuaTableDelegateDefinition LuaSceneComponentContainer::_initDef("init", "scene component init");
	const LuaTableDelegateDefinition LuaSceneComponentContainer::_shutdownDef("shutdown", "scene component shutdown");
	const LuaTableDelegateDefinition LuaSceneComponentContainer::_renderResetDef("render_reset", "scene component render reset");
	const LuaTableDelegateDefinition LuaSceneComponentContainer::_updateDef("update", "scene component update");


	void LuaSceneComponentContainer::init(Scene& scene, App& app)
	{
		for (auto& [type, comp] : _components)
		{
			_initDef(comp);
		}
	}

	void LuaSceneComponentContainer::shutdown()
	{
		for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
		{
			_shutdownDef(itr->second);
		}
	}

	void LuaSceneComponentContainer::renderReset()
	{
		for (auto& [type, comp] : _components)
		{
			_renderResetDef(comp);
		}
	}

	void LuaSceneComponentContainer::update(float deltaTime)
	{
		for (auto& [type, comp] : _components)
		{
			_updateDef(comp, deltaTime);
		}
	}
}