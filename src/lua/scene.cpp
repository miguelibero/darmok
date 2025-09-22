#include "lua/scene.hpp"
#include "lua/transform.hpp"
#include "lua/camera.hpp"
#include "lua/light.hpp"
#include "lua/render_scene.hpp"
#include "lua/scene_filter.hpp"
#include "lua/scene_serialize.hpp"
#include "lua/freelook.hpp"
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/string.hpp>
#include <darmok/transform.hpp>

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#endif

#ifdef DARMOK_JOLT
#include "physics3d.hpp"
#include "character.hpp"
#endif

namespace darmok
{
	LuaEntityComponent::LuaEntityComponent(const sol::table& table) noexcept
		: _table{ table }
	{
	}

	entt::id_type LuaEntityComponent::getType() const noexcept
	{
		return LuaUtils::getTypeId(_table).value();
	}

	sol::object LuaEntityComponent::getReal() const noexcept
	{
		return _table;
	}

	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept
		: _entity{ entity }
		, _scene{ scene }
	{
	}

	std::string LuaEntity::toString() const noexcept
	{
		std::string str{ "Entity(" + std::to_string(_entity) };
		if (!isValid())
		{
			str += " invalid";
		}
		return str + ")";
	}

	bool LuaEntity::isValid() const noexcept
	{
		if (auto scene = _scene.lock())
		{
			return scene->isValidEntity(_entity);
		}
		return false;
	}

	std::shared_ptr<Scene> LuaEntity::getScene() const
	{
		if (auto scene = _scene.lock())
		{
			return scene;
		}
		throw std::runtime_error{ "scene expired" };
	}

	bool LuaEntity::operator==(const LuaEntity& other) const noexcept
	{
		if (_entity != other._entity)
		{
			return false;
		}
		if (auto scene = _scene.lock())
		{
			if (auto otherScene = other._scene.lock())
			{
				return scene == otherScene;
			}
		}
		return false;
	}

	bool LuaEntity::operator!=(const LuaEntity& other) const noexcept
	{
		return !operator==(other);
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
	}

	bool LuaEntity::removeComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				return scene->removeComponent(_entity, typeId.value());
			}
		}
		return false;
	}

	bool LuaEntity::hasComponent(const sol::object& type) const noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				return scene->hasComponent(_entity, typeId.value());
			}
		}
		return false;
	}

	void LuaEntity::addLuaComponent(const sol::table& comp)
	{
		auto typeId = LuaUtils::getTypeId(comp).value();
		getScene()->addCustomComponent<LuaEntityComponent>(_entity, typeId, comp);
	}

	sol::object LuaEntity::getLuaComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				auto comp = scene->getCustomComponent<LuaEntityComponent>(_entity, typeId.value());
				if (comp)
				{
					return comp->getReal();
				}
			}
		}
		return sol::nil;
	}

	bool LuaEntity::checkForEachResult(const std::string& desc, const sol::protected_function_result& result)
	{
		LuaUtils::checkResult(desc, result);
		sol::object obj = result;
		auto type = obj.get_type();
		if (type == sol::type::boolean)
		{
			return obj.as<bool>();
		}
		return type != sol::type::nil;
	}

	bool LuaEntity::forEachChild(const sol::protected_function& callback)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return false;
		}
		return scene->forEachChild(_entity, [callback, scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntity(entity, scene), trans);
			return checkForEachResult("for each entity child", result);
		});
	}

	bool LuaEntity::forEachParent(const sol::protected_function& callback)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return false;
		}
		return scene->forEachParent(_entity, [callback, scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntity(entity, scene), trans);
			return checkForEachResult("for each entity parent", result);
		});
	}

	void LuaEntity::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::no_constructor,
			"scene", sol::property(&LuaEntity::getScene),
			"remove_component", &LuaEntity::removeComponent,
			"has_component", &LuaEntity::hasComponent,
			"add_lua_component", &LuaEntity::addLuaComponent,
			"get_lua_component", &LuaEntity::getLuaComponent,
			"for_each_child", &LuaEntity::forEachChild,
			"for_each_parent", &LuaEntity::forEachParent,
			sol::meta_function::to_string, &LuaEntity::toString
		);
	}

	bool LuaScene::removeSceneComponent(Scene& scene, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return scene.removeSceneComponent(typeId.value());
		}
		return false;
	}

	bool LuaScene::hasSceneComponent(const Scene& scene, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return scene.hasSceneComponent(typeId.value());
		}
		return false;
	}

	void LuaScene::addLuaSceneComponent(const std::shared_ptr<Scene>& scene, const sol::table& table)
	{
		auto comp = std::make_unique<LuaSceneComponent>(table, scene);
		scene->addSceneComponent(std::move(comp));
	}

	sol::object LuaScene::getLuaSceneComponent(Scene& scene, const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto comp = scene.getSceneComponent(typeId.value()))
			{
				auto& luaComp = static_cast<LuaSceneComponent&>(*comp);
				return luaComp.getReal();
			}
		}
		return sol::nil;
	}

	LuaEntity LuaScene::createEntity1(const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaEntity(scene->createEntity(), scene);
	}

	LuaEntity LuaScene::createEntity2(const std::shared_ptr<Scene>& scene, LuaEntity& parent) noexcept
	{
		auto entity = scene->createEntity();
		auto& parentTrans = scene->getOrAddComponent<Transform>(parent.getReal());
		scene->addComponent<Transform>(entity).setParent(parentTrans);
		return LuaEntity(entity, scene);
	}

	LuaEntity LuaScene::createEntity3(const std::shared_ptr<Scene>& scene, Transform& parent) noexcept
	{
		auto entity = scene->createEntity();
		scene->addComponent<Transform>(entity).setParent(parent);
		return LuaEntity(entity, scene);
	}

	LuaEntity LuaScene::createEntity4(const std::shared_ptr<Scene>& scene, LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		auto& parentTrans = scene->getOrAddComponent<Transform>(parent.getReal());
		auto& trans = scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		trans.setParent(parentTrans);
		return LuaEntity(entity, scene);
	}

	LuaEntity LuaScene::createEntity5(const std::shared_ptr<Scene>& scene, Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		auto& trans = scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		trans.setParent(parent);
		return LuaEntity(entity, scene);
	}

	LuaEntity LuaScene::createEntity6(const std::shared_ptr<Scene>& scene, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		return LuaEntity(entity, scene);
	}

	void LuaScene::destroyEntity(Scene& scene, const LuaEntity& entity) noexcept
	{
		scene.destroyEntity(entity.getReal());
	}

	std::optional<LuaEntity> LuaScene::getEntity(const std::shared_ptr<Scene>& scene, const sol::object& comp) noexcept
	{
		// TODO: check why this does not work
		if (comp.get_type() != sol::type::userdata)
		{
			return std::nullopt;
		}
		auto typeId = LuaUtils::getTypeId(comp);
		if (!typeId)
		{
			return std::nullopt;
		}
		auto entity = scene->getEntity(typeId.value(), comp.as<void*>());
		if (entity == entt::null)
		{
			return std::nullopt;
		}
		return LuaEntity(entity, scene);
	}

	std::optional<Viewport> LuaScene::getViewport(const Scene& scene) noexcept
	{
		return scene.getViewport();
	}

	void LuaScene::setViewport(Scene& scene, std::optional<VarViewport> vp) noexcept
	{
		if (vp)
		{
			scene.setViewport(LuaViewport::tableGet(vp));
		}
		else
		{
			scene.setViewport(sol::nullopt);
		}
	}

	void LuaScene::setUpdateFilter(Scene& scene, const sol::object& filter) noexcept
	{
		scene.setUpdateFilter(LuaEntityFilter::create(filter));
	}

	bool LuaScene::forEachEntity(const std::shared_ptr<Scene>& scene, const sol::protected_function& callback)
	{
		return scene->forEachEntity([callback, scene](auto entity) -> bool {
			auto result = callback(LuaEntity(entity, scene));
			return LuaEntity::checkForEachResult("for each entity", result);
		});
	}

	void LuaScene::bind(sol::state_view& lua) noexcept
	{
		LuaEntityFilter::bind(lua);
		LuaSceneAppComponent::bind(lua);
		LuaSceneConverter::bind(lua);
		LuaTransform::bind(lua);
		LuaCamera::bind(lua);
		LuaEntity::bind(lua);
		LuaRenderable::bind(lua);
		LuaFreelookController::bind(lua);

#ifdef DARMOK_OZZ
		LuaSkeletalAnimator::bind(lua);
		LuaRenderableSkeleton::bind(lua);
		LuaSkeletalAnimationSceneComponent::bind(lua);
#endif

#ifdef DARMOK_JOLT
		physics3d::LuaPhysicsSystem::bind(lua);
		physics3d::LuaPhysicsBody::bind(lua);
		physics3d::LuaCharacterController::bind(lua);
#endif

		lua.new_usertype<Scene>("Scene", sol::constructors<Scene(App&)>(),
			"create_entity", sol::overload(
				&LuaScene::createEntity1, &LuaScene::createEntity2,
				&LuaScene::createEntity3, &LuaScene::createEntity4,
				&LuaScene::createEntity5, &LuaScene::createEntity6),
			"destroy_entity", &LuaScene::destroyEntity,
			"get_entity", sol::resolve<std::optional<LuaEntity>(const std::shared_ptr<Scene>&, const sol::object&)>(&LuaScene::getEntity),
			sol::meta_function::to_string, &Scene::toString,
			"has_component", &LuaScene::hasSceneComponent,
			"remove_component", &LuaScene::removeSceneComponent,
			"add_lua_component", &LuaScene::addLuaSceneComponent,
			"get_lua_component", &LuaScene::getLuaSceneComponent,
			"for_each_entity", &LuaScene::forEachEntity,
			"viewport", sol::property(&LuaScene::getViewport, &LuaScene::setViewport),
			"current_viewport", sol::property(&Scene::getCurrentViewport),
			"render_chain", sol::property(sol::resolve<RenderChain&()>(&Scene::getRenderChain)),
			"name", sol::property(&Scene::getName, &Scene::setName),
			"paused", sol::property(&Scene::isPaused, &Scene::setPaused),
			"update_filter", sol::property(&Scene::getUpdateFilter, &LuaScene::setUpdateFilter)
		);
	}

	LuaSceneComponent::LuaSceneComponent(const sol::table& table, const std::weak_ptr<Scene>& scene) noexcept
		: _table(table)
		, _scene(scene)
	{
	}

	sol::object LuaSceneComponent::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaSceneComponent::_initDef("init", "scene component init");
	const LuaTableDelegateDefinition LuaSceneComponent::_shutdownDef("shutdown", "scene component shutdown");
	const LuaTableDelegateDefinition LuaSceneComponent::_renderResetDef("render_reset", "scene component render reset");
	const LuaTableDelegateDefinition LuaSceneComponent::_updateDef("update", "scene component update");

	void LuaSceneComponent::init(Scene& scene, App& app)
	{
		if (auto scene = _scene.lock())
		{
			_initDef(_table, scene);
		}
	}

	void LuaSceneComponent::shutdown()
	{
		_shutdownDef(_table);
	}

	bgfx::ViewId LuaSceneComponent::renderReset(bgfx::ViewId viewId)
	{
		return _renderResetDef(_table).as<bgfx::ViewId>();
	}

	void LuaSceneComponent::update(float deltaTime)
	{
		_updateDef(_table, deltaTime);
	}

	SceneAppComponent& LuaSceneAppComponent::addAppComponent1(App& app) noexcept
	{
		return app.addComponent<SceneAppComponent>();
	}

	SceneAppComponent& LuaSceneAppComponent::addAppComponent2(App& app, const std::shared_ptr<Scene>& scene) noexcept
	{
		return app.addComponent<SceneAppComponent>(scene);
	}

	OptionalRef<SceneAppComponent>::std_t LuaSceneAppComponent::getAppComponent(App& app) noexcept
	{
		return app.getComponent<SceneAppComponent>();
	}

	std::shared_ptr<Scene> LuaSceneAppComponent::getScene1(const SceneAppComponent& comp) noexcept
	{
		return getScene2(comp, 0);
	}

	std::shared_ptr<Scene>LuaSceneAppComponent::getScene2(const SceneAppComponent& comp, size_t i) noexcept
	{
		return comp.getScene(i);
	}

	SceneAppComponent& LuaSceneAppComponent::setScene1(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene) noexcept
	{
		return setScene2(comp, scene, 0);
	}

	SceneAppComponent& LuaSceneAppComponent::setScene2(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene, size_t i) noexcept
	{
		comp.setScene(scene, i);
		return comp;
	}

	std::shared_ptr<Scene> LuaSceneAppComponent::addScene1(SceneAppComponent& comp) noexcept
	{
		return comp.addScene();
	}

	SceneAppComponent& LuaSceneAppComponent::addScene2(SceneAppComponent& comp, const std::shared_ptr<Scene>& scene) noexcept
	{
		comp.addScene(scene);
		return comp;
	}

	void LuaSceneAppComponent::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<SceneAppComponent>("SceneAppComponent",
			sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<SceneAppComponent>::value),
			"add_app_component", sol::overload(
				&LuaSceneAppComponent::addAppComponent1,
				&LuaSceneAppComponent::addAppComponent2
			),
			"get_app_component", &LuaSceneAppComponent::getAppComponent,
			"scene", sol::property(&LuaSceneAppComponent::getScene1, &LuaSceneAppComponent::setScene1),
			"get_scene", sol::overload(&LuaSceneAppComponent::getScene1, &LuaSceneAppComponent::getScene2),
			"set_scene", sol::overload(&LuaSceneAppComponent::setScene1, &LuaSceneAppComponent::setScene2),
			"add_scene", sol::overload(&LuaSceneAppComponent::addScene1, &LuaSceneAppComponent::addScene2)
		);
	}
}