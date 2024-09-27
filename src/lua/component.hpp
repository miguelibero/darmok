#pragma once

#include <sol/sol.hpp>
#include <unordered_map>
#include <string>
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include "utils.hpp"

namespace darmok
{
    class LuaComponentContainer
	{
	public:
		bool contains(const sol::object& type) const noexcept;
		void add(const sol::table& comp);
		bool remove(const sol::object& type) noexcept;
		sol::object get(const sol::object& type) noexcept;
	protected:
		using Key = const void*;
		using Components = std::vector<std::pair<Key, sol::table>>;
		Components _components;
		Components::iterator find(Key key) noexcept;
		Components::const_iterator find(Key key) const noexcept;
	private:
		static const std::string _typeKey;
		static Key getKey(const sol::object& type) noexcept;
	};

	class LuaAppComponentContainer : public LuaComponentContainer, public IAppComponent
	{
	public:
		void init(App& app) override;
		void shutdown() override;
		void renderReset() override;
		void update(float deltaTime) override;
	private:
		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};

	class LuaScene;

	class LuaSceneComponentContainer : public LuaComponentContainer, public ISceneComponent
	{
	public:
		LuaSceneComponentContainer(LuaScene& scene) noexcept;
		void init(Scene& scene, App& app) override;
		void shutdown() override;
		void renderReset() override;
		void update(float deltaTime) override;
	private:
		LuaScene& _scene;
		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};

}