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

	void LuaAppComponentContainer::init(App& app)
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "init", "app component init",
				[](auto& func, auto& self) {
					return func(self);
			});
		}
	}

	void LuaAppComponentContainer::shutdown()
	{
		for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
		{
			auto& comp = itr->second;
			LuaUtils::callTableDelegate(comp, "shutdown", "app component shutdown",
				[](auto& func, auto& self) {
					return func(self);
				});
		}
	}

	void LuaAppComponentContainer::renderReset()
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "render_reset", "app component render reset",
				[](auto& func, auto& self) {
					return func(self);
				});
		}
	}

	void LuaAppComponentContainer::update(float deltaTime)
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "update", "app component update",
				[deltaTime](auto& func, auto& self) {
					return func(self, deltaTime);
				});
		}
	}

	LuaSceneComponentContainer::LuaSceneComponentContainer(LuaScene& scene) noexcept
		: _scene(scene)
	{
	}

	void LuaSceneComponentContainer::init(Scene& scene, App& app)
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "init", "scene component init",
				[this](auto& func, auto& self) {
					return func(self, _scene);
				});
		}
	}

	void LuaSceneComponentContainer::shutdown()
	{
		for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
		{
			auto& comp = itr->second;
			LuaUtils::callTableDelegate(comp, "shutdown", "scene component shutdown",
				[](auto& func, auto& self) {
					return func(self);
				});
		}
	}

	void LuaSceneComponentContainer::renderReset()
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "render_reset", "scene component render reset",
				[](auto& func, auto& self) {
					return func(self);
				});
		}
	}

	void LuaSceneComponentContainer::update(float deltaTime)
	{
		for (auto& [type, comp] : _components)
		{
			LuaUtils::callTableDelegate(comp, "update", "scene component update",
				[deltaTime](auto& func, auto& self) {
					return func(self, deltaTime);
				});
		}
	}

}