#pragma once

#include "lua.hpp"
#include "transform.hpp"
#include "utils.hpp"
#include "viewport.hpp"

#include <darmok/color.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>

#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>

namespace darmok
{
	class LuaEntityComponent final
	{
	public:
		LuaEntityComponent(const sol::table& table) noexcept;
		entt::id_type getType() const noexcept;
		sol::object getReal() const noexcept;
	private:
		sol::main_table _table;
	};

	class LuaScene;
	class LuaSceneComponentContainer;

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept;
		
		const Entity& getReal() const noexcept;
		std::shared_ptr<Scene> getScene() const;

		bool operator==(const LuaEntity& other) const noexcept;
		bool operator!=(const LuaEntity& other) const noexcept;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			return getScene()->addComponent<T>(_entity, std::forward<Args>(args)...);
		}

		template<typename T>
		OptionalRef<T> getComponent() noexcept
		{
			if (auto scene = _scene.lock())
			{
				return scene->getComponent<T>(_entity);
			}
			return nullptr;
		}

		static void bind(sol::state_view& lua) noexcept;

		static bool checkForEachResult(const std::string& desc, const sol::protected_function_result& result);
	private:
		Entity _entity;
		std::weak_ptr<Scene> _scene;

		std::string toString() const noexcept;
		bool isValid() const noexcept;
		bool removeComponent(const sol::object& type) noexcept;
		bool hasComponent(const sol::object& type) const noexcept;
		void addLuaComponent(const sol::table& comp);
		sol::object getLuaComponent(const sol::object& type) noexcept;

		bool forEachChild(const sol::protected_function& callback);
		bool forEachParent(const sol::protected_function& callback);
	};

	class App;

	// TODO: make this class static, bind directly to darmok::scene
	class LuaScene final
	{
	public:
		template<typename T>
		static std::optional<LuaEntity> getEntity(const std::shared_ptr<Scene>& scene, const T& component) noexcept
		{
			auto entity = scene->getEntity(component);
			if (entity == entt::null)
			{
				return std::nullopt;
			}
			return LuaEntity(entity, scene);
		}

		static void bind(sol::state_view& lua) noexcept;
	private:
		static bool removeSceneComponent(Scene& scene, const sol::object& type);
		static bool hasSceneComponent(const Scene& scene, const sol::object& type);
		static void addLuaSceneComponent(const std::shared_ptr<Scene>& scene, const sol::table& table);
		static sol::object getLuaSceneComponent(Scene& scene, const sol::object& type) noexcept;

		static LuaEntity createEntity1(const std::shared_ptr<Scene>& scene) noexcept;
		static LuaEntity createEntity2(const std::shared_ptr<Scene>& scene, LuaEntity& parent) noexcept;
		static LuaEntity createEntity3(const std::shared_ptr<Scene>& scene, Transform& parent) noexcept;
		static LuaEntity createEntity4(const std::shared_ptr<Scene>& scene, LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		static LuaEntity createEntity5(const std::shared_ptr<Scene>& scene, Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		static LuaEntity createEntity6(const std::shared_ptr<Scene>& scene, const VarLuaTable<glm::vec3>& position) noexcept;
		static void destroyEntity(Scene& scene, const LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const std::shared_ptr<Scene>& scene, const sol::object& comp) noexcept;

		static void setUpdateFilter(Scene& scene, const sol::object& filter) noexcept;

		static bool forEachEntity(const std::shared_ptr<Scene>& scene, const sol::protected_function& callback);
	};

	class LuaSceneComponent final : public ITypeSceneComponent<LuaSceneComponent>
	{
	public:
		LuaSceneComponent(const sol::table& table, const std::weak_ptr<Scene>& scene) noexcept;
		sol::object getReal() const noexcept;

		void init(Scene& scene, App& app) override;
		void shutdown() override;
		bgfx::ViewId renderReset(bgfx::ViewId viewId) override;
		void update(float deltaTime) override;

	private:
		sol::main_table _table;
		std::weak_ptr<Scene> _scene;

		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};

	class SceneAppComponent;

	class LuaSceneAppComponent final : public IAppComponent
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:

		static SceneAppComponent& addAppComponent1(App& app) noexcept;
		static SceneAppComponent& addAppComponent2(App& app, const std::shared_ptr<Scene>& scene) noexcept;
		static OptionalRef<SceneAppComponent>::std_t getAppComponent(App& app) noexcept;
		static std::shared_ptr<Scene> getScene1(const SceneAppComponent& comp) noexcept;
		static std::shared_ptr<Scene> getScene2(const SceneAppComponent& comp, size_t i) noexcept;
		static std::shared_ptr<Scene> addScene1(SceneAppComponent& comp) noexcept;
		static SceneAppComponent& addScene2(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene) noexcept;
		static SceneAppComponent& setScene1(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene) noexcept;
		static SceneAppComponent& setScene2(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene, size_t i) noexcept;
	};
}