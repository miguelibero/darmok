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
#include "viewport.hpp"

namespace darmok
{
	class LuaScene;
	class LuaComponentContainer;
	class LuaSceneComponentContainer;

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

		static void bind(sol::state_view& lua) noexcept;

	private:
		Entity _entity;
		std::weak_ptr<Scene> _scene;
		mutable OptionalRef<LuaComponentContainer> _luaComponents;

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
		bool tryGetLuaComponentContainer() const noexcept;

		bool forEachChild(const sol::protected_function& callback);
		bool forEachParent(const sol::protected_function& callback);
		static bool checkForEachResult(const std::string& desc, const sol::protected_function_result& result);
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
		mutable OptionalRef<LuaSceneComponentContainer> _luaComponents;

		bool removeSceneComponent(const sol::object& type);
		bool hasSceneComponent(const sol::object& type) const;
		bool hasLuaSceneComponent(const sol::object& type) const noexcept;
		void addLuaSceneComponent(const sol::table& comp);
		bool removeLuaSceneComponent(const sol::object& type) noexcept;
		sol::object getLuaSceneComponent(const sol::object& type) noexcept;
		bool tryGetLuaComponentContainer() const noexcept;

		std::string toString() const noexcept;
		EntityRegistry& getRegistry() noexcept;
		LuaEntity createEntity1() noexcept;
		LuaEntity createEntity2(LuaEntity& parent) noexcept;
		LuaEntity createEntity3(Transform& parent) noexcept;
		LuaEntity createEntity4(LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity5(Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity6(const VarLuaTable<glm::vec3>& position) noexcept;

		bool destroyEntity(const LuaEntity& entity) noexcept;

		std::optional<Viewport> getViewport() const noexcept;
		void setViewport(std::optional<VarViewport> vp) noexcept;
		Viewport getCurrentViewport() noexcept;
		RenderChain& getRenderChain() noexcept;

		void setName(const std::string& name) noexcept;
		const std::string& getName() const noexcept;
	};

	class SceneAppComponent;

	class LuaSceneAppComponent final : public IAppComponent
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static SceneAppComponent& addAppComponent1(LuaApp& app) noexcept;
		static SceneAppComponent& addAppComponent2(LuaApp& app, const LuaScene& scene) noexcept;
		static std::optional<LuaScene> getScene1(const SceneAppComponent& comp) noexcept;
		static std::optional<LuaScene> getScene2(const SceneAppComponent& comp, size_t i) noexcept;
		static LuaScene addScene1(SceneAppComponent& comp) noexcept;
		static SceneAppComponent& addScene2(SceneAppComponent& comp, LuaScene& scene) noexcept;
		static SceneAppComponent& setScene1(SceneAppComponent& comp, const LuaScene& scene) noexcept;
		static SceneAppComponent& setScene2(SceneAppComponent& comp, const LuaScene& scene, size_t i) noexcept;
	};
}