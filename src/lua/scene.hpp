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
		LuaScene getScene() const;
		std::weak_ptr<Scene> getWeakScene() const noexcept;

		bool operator==(const LuaEntity& other) const noexcept;
		bool operator!=(const LuaEntity& other) const noexcept;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			return getRealScene().addComponent<T>(_entity, std::forward<Args>(args)...);
		}

		template<typename T>
		OptionalRef<T> getComponent()
		{
			return getRealScene().getComponent<T>(_entity);
		}

		static void bind(sol::state_view& lua) noexcept;

		static bool checkForEachResult(const std::string& desc, const sol::protected_function_result& result);
	private:
		Entity _entity;
		std::weak_ptr<Scene> _scene;

		Scene& getRealScene();
		const Scene& getRealScene() const;

		std::string toString() const noexcept;
		bool isValid() const noexcept;
		bool removeComponent(const sol::object& type);
		bool hasComponent(const sol::object& type) const;
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
		LuaScene(const std::shared_ptr<Scene>& scene) noexcept;
		LuaScene(App& app) noexcept;

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

		bool removeSceneComponent(const sol::object& type);
		bool hasSceneComponent(const sol::object& type) const;
		void addLuaSceneComponent(const sol::table& table);
		sol::object getLuaSceneComponent(const sol::object& type) noexcept;

		std::string toString() const noexcept;
		LuaEntity createEntity1() noexcept;
		LuaEntity createEntity2(LuaEntity& parent) noexcept;
		LuaEntity createEntity3(Transform& parent) noexcept;
		LuaEntity createEntity4(LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity5(Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept;
		LuaEntity createEntity6(const VarLuaTable<glm::vec3>& position) noexcept;

		void destroyEntity(const LuaEntity& entity) noexcept;

		std::optional<Viewport> getViewport() const noexcept;
		void setViewport(std::optional<VarViewport> vp) noexcept;
		Viewport getCurrentViewport() noexcept;
		RenderChain& getRenderChain() noexcept;

		void setName(const std::string& name) noexcept;
		const std::string& getName() const noexcept;

		void setPaused(bool paused) noexcept;
		bool getPaused() const noexcept;

		const TypeFilter& getUpdateFilter() const noexcept;
		void setUpdateFilter(const sol::object& filter) noexcept;

		bool forEachEntity(const sol::protected_function& callback);
	};

	class LuaSceneComponent final : public ITypeSceneComponent<LuaSceneComponent>
	{
	public:
		LuaSceneComponent(const sol::table& table, const std::weak_ptr<Scene>& scene) noexcept;
		sol::object getReal() const noexcept;

		void init(Scene& scene, App& app) override;
		void shutdown() override;
		void renderReset() override;
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
		static SceneAppComponent& addAppComponent2(App& app, const LuaScene& scene) noexcept;
		static OptionalRef<SceneAppComponent>::std_t getAppComponent(App& app) noexcept;
		static std::optional<LuaScene> getScene1(const SceneAppComponent& comp) noexcept;
		static std::optional<LuaScene> getScene2(const SceneAppComponent& comp, size_t i) noexcept;
		static LuaScene addScene1(SceneAppComponent& comp) noexcept;
		static SceneAppComponent& addScene2(SceneAppComponent& comp, LuaScene& scene) noexcept;
		static SceneAppComponent& setScene1(SceneAppComponent& comp, const LuaScene& scene) noexcept;
		static SceneAppComponent& setScene2(SceneAppComponent& comp, const LuaScene& scene, size_t i) noexcept;
	};
}