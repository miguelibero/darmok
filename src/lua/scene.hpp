#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <darmok/color.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <sol/sol.hpp>

#include "transform.hpp"
#include "utils.hpp"

namespace darmok
{
	class LuaScene;

	class LuaComponent final
	{
	public:
		bool hasComponent(const sol::object& type) const noexcept;
		void addComponent(const sol::table& comp);
		bool removeComponent(const sol::object& type) noexcept;
		sol::object getComponent(const sol::object& type) noexcept;
	private:
		using Key = const void*;
		std::unordered_map<Key, sol::table> _components;
		static const std::string _typeKey;
		static Key getKey(const sol::object& type) noexcept;
	};

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept;
		
		const Entity& getReal() const noexcept;
		LuaScene getScene() const;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) noexcept
		{
			return getRegistry().emplace<T>(_entity, std::forward<Args>(args)...);
		}

		template<typename T, typename R, typename... Args>
		T& addWrapperComponent(Args&&... args) noexcept
		{
			auto& real = addComponent<R>(std::forward<Args>(args)...);
			return addComponent<T>(real);
		}

		template<typename T>
		OptionalRef<T> getComponent()
		{
			return getRegistry().try_get<T>(_entity);
		}

		template<typename T, typename L, typename... Args>
		std::optional<L> getComponent(Args&&... args)
		{
			auto ptr = getRegistry().try_get<T>(_entity);
			if (ptr == nullptr)
			{
				return std::nullopt;
			}
			return L(*ptr, std::forward<Args>(args)...);
		}

		static void bind(sol::state_view& lua) noexcept;

	private:
		Entity _entity;
		std::weak_ptr<Scene> _scene;
		OptionalRef<LuaComponent> _lua;

		EntityRegistry& getRegistry();
		const EntityRegistry& getRegistry() const;

		std::string toString() const noexcept;
		bool isValid() const noexcept;
		bool removeComponent(const sol::object& type);
		bool hasComponent(const sol::object& type) const;

		bool hasLuaComponent(const sol::object& type) const noexcept;
		void addLuaComponent(const sol::table& comp);
		bool removeLuaComponent(const sol::object& type) noexcept;
		sol::object getLuaComponent(const sol::object& type) noexcept;
	};

	class LuaApp;

	class LuaScene final
	{
	public:
		LuaScene(const std::shared_ptr<Scene>& scene) noexcept;
		LuaScene(LuaApp& app) noexcept;

		template<typename T, typename R, typename... A>
		T& addSceneComponent(A&&... args)
		{
			auto& real = getReal()->addSceneComponent<R>(std::forward<A>(args)...);
			return getReal()->addSceneComponent<T>(real);
		}

		std::string toString() const noexcept;
		EntityRegistry& getRegistry() noexcept;
		LuaEntity createEntity1() noexcept;
		LuaEntity createEntity2(LuaEntity& parent) noexcept;
		LuaEntity createEntity3(Transform& parent) noexcept;
		LuaEntity createEntity4(LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity5(Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity6(const VarLuaTable<glm::vec3>& position) noexcept;

		bool destroyEntity(const LuaEntity& entity) noexcept;

		template<typename T>
		std::optional<LuaEntity> getEntity(const T& component) noexcept
		{
			auto entity = _scene->getEntity(component);
			if (entity == entt::null)
			{
				return std::nullopt;
			}
			return LuaEntity(entity, _scene);
		}

		const std::shared_ptr<Scene>& getReal() const noexcept;
		std::shared_ptr<Scene>& getReal() noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Scene> _scene;
	};

	class SceneAppComponent;

	class LuaSceneAppComponent final : public IAppComponent
	{
	public:
		LuaSceneAppComponent(SceneAppComponent& comp) noexcept;
		static LuaSceneAppComponent& addAppComponent1(LuaApp& app) noexcept;
		static LuaSceneAppComponent& addAppComponent2(LuaApp& app, const LuaScene& scene) noexcept;
		LuaScene getScene() const noexcept;
		LuaSceneAppComponent& setScene(const LuaScene& scene) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<SceneAppComponent> _comp;
	};
}